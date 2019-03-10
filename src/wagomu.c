/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* Copyright (C) 2009 The Tegaki project contributors
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/* 
* Contributors to this file:
*  - Mathieu Blondel
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <inttypes.h>
#include <malloc.h>
//#include <glib/gstdio.h>
//#include <glib.h>

#include "wagomu.h"

#define MAGIC_NUMBER 0x77778888
#define VEC_DIM_MAX 4

#undef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#undef MIN3
#define MIN3(a,b,c) (MIN((a),MIN((b),(c))))

#undef MIN4
#define MIN4(a,b,c,d) (MIN((a),MIN3((b),(c),(d))))

#undef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#undef MAX3
#define MAX3(a,b,c) (MAX((a),MAX((b),(c))))

#undef MAX4
#define MAX4(a,b,c,d) (MAX((a),MAX3((b),(c),(d))))

#ifdef __SSE__

#undef MIN3VEC
#define MIN3VEC(a,b,c) (_mm_min_ps((a),_mm_min_ps((b),(c))))

#endif

#undef SWAP
#define SWAP(a,b,tmp) tmp = a; a = b; b = tmp

#define WAGOMU_RECOGNIZER_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), WAGOMU_TYPE_RECOGNIZER, WagomuRecognizerPrivate))

typedef struct _WagomuRecognizerPrivate WagomuRecognizerPrivate;


struct _WagomuRecognizerPrivate
{
    GMappedFile *file;
    char *data;

    guint n_characters;
    guint n_groups;
    /* dimension contains the actual vector dimension, e.g 2,
       while VECTOR_DIMENSION_MAX contains the vector dimension plus some
       padding, e.g. 4 */
    guint dimension;
    guint downsample_threshold;

    CharacterInfo *characters;
    CharacterGroup *groups;
    gfloat *strokedata;

#ifdef __SSE__
    wg_v4sf *dtw1v;
    wg_v4sf *dtw2v;
#endif

    gfloat *dtw1;
    gfloat *dtw2;

    char *error_msg;

    CharDist *distm;

    guint window_size;
};


G_DEFINE_TYPE (WagomuRecognizer, wagomu_recognizer, G_TYPE_OBJECT)

static void wagomu_recognizer_dispose (GObject *object);


static void
wagomu_recognizer_class_init (WagomuRecognizerClass *klass)
{
    GObjectClass *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose = wagomu_recognizer_dispose;

    g_type_class_add_private (gobject_class, sizeof (WagomuRecognizerPrivate));
}

static void
wagomu_recognizer_init (WagomuRecognizer *recognizer)
{
    WagomuRecognizerPrivate *priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_if_fail (priv);
    priv->window_size = 3;
}

static void
wagomu_recognizer_dispose (GObject *object)
{
    WagomuRecognizer *recognizer = WAGOMU_RECOGNIZER (object);

    wagomu_recognizer_clear (recognizer);

    G_OBJECT_CLASS (wagomu_recognizer_parent_class)->dispose (object);
}


WagomuRecognizer *
wagomu_recognizer_new (void)
{
    WagomuRecognizer *recognizer = g_object_new(WAGOMU_TYPE_RECOGNIZER, NULL);
    return recognizer;
}

static guint
wagomu_recognize_get_max_n_vectors(WagomuRecognizer *recognizer) 
{
    
    guint i, max_n_vectors;
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_val_if_fail (priv, 0);

    for (i=0, max_n_vectors=0; i < priv->n_characters; i++)
        if (priv->characters[i].n_vectors > max_n_vectors)
            max_n_vectors = priv->characters[i].n_vectors;

    return max_n_vectors;
}


gboolean
wagomu_recognizer_open(WagomuRecognizer *recognizer, char* path)
{
    
    guint *header;
    char *cursor;
    guint max_n_vectors;
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_val_if_fail (priv, FALSE);




    priv->file = g_mapped_file_new(path, FALSE, NULL);

    if (!priv->file) {
        priv->error_msg = (char *) "Couldn't map file";
        return FALSE;
    }

    priv->data = g_mapped_file_get_contents(priv->file);

    header = (unsigned int *)priv->data;

    if (header[0] != MAGIC_NUMBER) {
        priv->error_msg = (char *) "Not a valid file";
        return FALSE;
    }

    priv->n_characters =  header[1];
    priv->n_groups = header[2];
    priv->dimension = header[3];
    priv->downsample_threshold = header[4];

    if (priv->n_characters == 0 || priv->n_groups == 0) {
        priv->error_msg = (char *) "No characters in this model";
        return FALSE;
    }
    
    cursor = priv->data + 5 * sizeof(unsigned int);
    priv->characters = (CharacterInfo *)cursor;

    cursor += priv->n_characters * sizeof(CharacterInfo);
    priv->groups = (CharacterGroup *)cursor;

    priv->strokedata = (float *)(priv->data + priv->groups[0].offset);

    priv->distm = (CharDist *) malloc(priv->n_characters * sizeof(CharDist));

    max_n_vectors = wagomu_recognize_get_max_n_vectors(recognizer);

#ifdef __SSE__
    priv->dtw1v = (wg_v4sf *) memalign(16, max_n_vectors * VEC_DIM_MAX *
                                     sizeof(wg_v4sf));
    priv->dtw2v = (wg_v4sf *) memalign(16, max_n_vectors * VEC_DIM_MAX *
                                     sizeof(wg_v4sf));
    priv->dtw1 = (float *) priv->dtw1v;
    priv->dtw2 = (float *) priv->dtw2v;
#else
    priv->dtw1 = (float *) memalign(16, max_n_vectors * VEC_DIM_MAX *
                                  sizeof(float));
    priv->dtw2 = (float *) memalign(16, max_n_vectors * VEC_DIM_MAX *
                                  sizeof(float));
#endif

    return TRUE;
}

guint
wagomu_recognizer_get_n_characters(WagomuRecognizer *recognizer)
{
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_val_if_fail (priv, 0);
    
    return priv->n_characters;
}

guint
wagomu_recognizer_get_dimension(WagomuRecognizer *recognizer)
{
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_val_if_fail (priv, 0);
    
    return priv->dimension;
}



void
wagomu_recognizer_clear(WagomuRecognizer  *recognizer)
{
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_if_fail (priv);

    if (priv->file) g_mapped_file_free(priv->file);
    if (priv->distm) free(priv->distm);
    if (priv->dtw1) free(priv->dtw1);
    if (priv->dtw2) free(priv->dtw2);
}

static int char_dist_cmp(CharDist *a, CharDist *b) {
    if (a->dist < b->dist) return -1;
    if (a->dist > b->dist) return 1;
    return  0;
}

inline static gfloat wagomu_recognizer_local_distance(WagomuRecognizer *recognizer, gfloat *v1, gfloat *v2)
{
    
    gfloat sum = 0;
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_val_if_fail (priv, 0.0);
#ifdef __SSE__
    wg_v4sf res, v_;
    v_.v = _mm_set_ps1(-0.0);
    
    res.v = _mm_sub_ps(((wg_v4sf *)v2)->v, ((wg_v4sf *)v1)->v);
    res.v = _mm_andnot_ps(v_.v,res.v); // absolute value
    
    unsigned int i;
    for (i=0; i < priv->dimension; i++)
        sum += res.s[i];
#else
    unsigned int i;
    for (i=0; i < priv->dimension; i++) 
        sum += fabs(v2[i] - v1[i]);
#endif
    return sum;
}


inline static gfloat dtw(WagomuRecognizer *recognizer, gfloat *s, guint n, gfloat *t, guint m) 
{
    /*
    Compare an input sequence with a reference sequence.

    s: input sequence
    n: number of vectors in s

    t: reference sequence
    m: number of vectors in t
    */
    guint i, j;
    gfloat cost;
    gfloat *t_start, *tmp;
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_val_if_fail (priv, 0.0);

    t_start = t;

    /* Initialize the edge cells */
    for (i=1; i < m; i++)
        priv->dtw1[i] = FLT_MAX;

    priv->dtw1[0] = 0;
    priv->dtw2[0] = FLT_MAX;

    s += VEC_DIM_MAX;
   
    /* Iterate over columns */
    for (i=1; i < n; i++) {
        t = t_start + VEC_DIM_MAX;

        /* Iterate over cells of that column */
        for (j=1; j < m; j++) {
            cost = wagomu_recognizer_local_distance(recognizer, s, t);
            /* Inductive step */
            priv->dtw2[j] = cost + MIN3(priv->dtw2[j-1],priv->dtw1[j],priv->dtw1[j-1]);

            t += VEC_DIM_MAX;
        }

        SWAP(priv->dtw1,priv->dtw2,tmp);
        *(priv->dtw2) = FLT_MAX;

        s += VEC_DIM_MAX;
    }

    return priv->dtw1[m-1];
}


/*

m [X][ ][ ][ ][ ][r]
  [X][ ][ ][ ][ ][ ]
  [X][ ][ ][ ][ ][ ]
  [X][ ][ ][ ][ ][ ]
  [0][X][X][X][X][X]
                  n

Each cell in the n*m matrix is defined as follows:
    
    dtw(i,j) = local_distance(i,j) + MIN3(dtw(i-1,j-1), dtw(i-1,j), dtw(i,j-1))

Cells marked with an X are set to infinity.
The bottom-left cell is set to 0.
The top-right cell is the result.

At any given time, we only need two columns of the matrix, thus we use
two arrays dtw1 and dtw2 as our data structure.

[   ]   [   ]
[ j ]   [ j ]
[j-1]   [j-1]
[   ]   [   ]
[ X ]   [ X ]
dtw1    dtw2

A cell can thus be calculated as follows:

    dtw2(j) = local_distance(i,j) + MIN3(dtw2(j-1), dtw1(j), dtw1(j-1))

*/



#ifdef __SSE__
inline static wg_v4sf local_distance4(float *s,
                                           float *t0,
                                           float *t1,
                                           float *t2,
                                           float *t3) {

    wg_v4sf v_, v0, v1, v2, v3;
    v_.v = _mm_set_ps1(-0.0);
    v0.v = _mm_sub_ps(((wg_v4sf *)t0)->v, ((wg_v4sf *)s)->v);
    v0.v = _mm_andnot_ps(v_.v,v0.v); // absolute value
    v1.v = _mm_sub_ps(((wg_v4sf *)t1)->v, ((wg_v4sf *)s)->v);
    v1.v = _mm_andnot_ps(v_.v,v1.v); // absolute value
    v2.v = _mm_sub_ps(((wg_v4sf *)t2)->v, ((wg_v4sf *)s)->v);
    v2.v = _mm_andnot_ps(v_.v,v2.v); // absolute value
    v3.v = _mm_sub_ps(((wg_v4sf *)t3)->v, ((wg_v4sf *)s)->v);
    v3.v = _mm_andnot_ps(v_.v,v3.v); // absolute value
    // convert row vectors to column vectors
    _MM_TRANSPOSE4_PS(v0.v, v1.v, v2.v, v3.v);
    v3.v = _mm_add_ps(v3.v, v2.v);
    v3.v = _mm_add_ps(v3.v, v1.v);
    v3.v = _mm_add_ps(v3.v, v0.v);
    return v3;
}

#define DTW4_PROCESS_REMAINING(n, m, t) \
do { \
    for (j=common; j < m; j++) { \
        costf = wagomu_recognizer_local_distance(recognizer, s, t); \
        priv->dtw2v[j].s[n] = costf + MIN3(priv->dtw2v[j-1].s[n], \
                                     priv->dtw1v[j].s[n], \
                                     priv->dtw1v[j-1].s[n]); \
        t += VEC_DIM_MAX; \
    } \
} while(0)


inline static wg_v4sf dtw4(WagomuRecognizer *recognizer, float *s, unsigned int n, 
                                float *t0, unsigned int m0,
                                float *t1, unsigned int m1,
                                float *t2, unsigned int m2,
                                float *t3, unsigned int m3) {
    /*
    Compare an input sequence with 4 reference sequences.

    For one column of the DTW matrix, MIN4(m0,m1,m2,m3) cells are calculated
    using vector instructions. The rest of the cells are calculated
    sequentially.

    s: input sequence
    n: number of vectors in s

    t0..t3: reference sequences
    m0..m3: number of vectors in the sequence
    */
    unsigned int i, j, common;
    wg_v4sf cost;
    float costf;
    float *t_start0, *t_start1, *t_start2, *t_start3;
    wg_v4sf *tmp;
    wg_v4sf res;
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);

    t_start0 = t0; t_start1 = t1; t_start2 = t2; t_start3 = t3;

    /* Initialize the edge cells */
    priv->dtw1v[0].v = _mm_set_ps1(0);
    priv->dtw2v[0].v = _mm_set_ps1(FLT_MAX);

    for (i=1; i < MAX4(m0,m1,m2,m3); i++)
        priv->dtw1v[i].v = _mm_set_ps1(FLT_MAX);

    s += VEC_DIM_MAX;
   
    common = MIN4(m0,m1,m2,m3);

    /* Iterate over columns */
    for (i=1; i < n; i++) {
        t0 = t_start0 + VEC_DIM_MAX; t1 = t_start1 + VEC_DIM_MAX;
        t2 = t_start2 + VEC_DIM_MAX; t3 = t_start3 + VEC_DIM_MAX;

        /* Iterate over cells of that column */
        /* Process 4 cells at a time in parallel */
        for (j=1; j < common; j++) {
            cost = local_distance4(s, t0, t1, t2, t3);
            /* Inductive step */
            priv->dtw2v[j].v = _mm_add_ps(cost.v,
                                MIN3VEC(priv->dtw2v[j-1].v,priv->dtw1v[j].v,priv->dtw1v[j-1].v));

            t0 += VEC_DIM_MAX; t1 += VEC_DIM_MAX;
            t2 += VEC_DIM_MAX; t3 += VEC_DIM_MAX;
        }

        /* The remaining of cells is calculated sequentially */
        DTW4_PROCESS_REMAINING(0, m0, t0);
        DTW4_PROCESS_REMAINING(1, m1, t1);
        DTW4_PROCESS_REMAINING(2, m2, t2);
        DTW4_PROCESS_REMAINING(3, m3, t3);

        SWAP(priv->dtw1v,priv->dtw2v,tmp);
        priv->dtw2v[0].v = _mm_set_ps1(FLT_MAX);

        s += VEC_DIM_MAX;
    }

    res.s[0] = priv->dtw1v[m0-1].s[0]; res.s[1] = priv->dtw1v[m1-1].s[1];
    res.s[2] = priv->dtw1v[m2-1].s[2]; res.s[3] = priv->dtw1v[m3-1].s[3];
    
    return res;
}
#endif


WagomuResults * wagomu_recognizer_recognize(WagomuRecognizer *recognizer, WagomuCharacter *ch, guint n_results)
{

    unsigned int group_id, i, size, n_chars, char_id, n_group_chars;
    unsigned int n_vectors, n_strokes;
    float *cursor;
    float *input;
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_val_if_fail (priv, NULL);
    
    cursor = priv->strokedata;

    n_vectors = wagomu_character_get_n_vectors(ch);
    n_strokes = wagomu_character_get_n_strokes(ch);
    input = wagomu_character_get_points(ch);

    #if 0
    assert_aligned16((char *) input);
    #endif

    for (group_id=0, n_chars=0, char_id=0; group_id < priv->n_groups; group_id++) {
        /* Only compare the input with templates which have
           +- window_size the same number of strokes as the input */
        if (n_strokes > priv->window_size) {
            if (priv->groups[group_id].n_strokes > (n_strokes + priv->window_size))
                break;

            if (priv->groups[group_id].n_strokes < (n_strokes - priv->window_size)) {
                char_id += priv->groups[group_id].n_chars;
                continue;
            }
        }

        cursor = (float *) (priv->data + priv->groups[group_id].offset);



#ifdef __SSE__
        float *ref1, *ref2, *ref3, *ref4;
        unsigned int size1, size2, size3, size4;
        wg_v4sf dtwres4;

        /* Process 4 reference characters at a time */
        for (i=0; i < (priv->groups[group_id].n_chars / 4); i++) {
            priv->distm[n_chars].unicode = priv->characters[char_id].unicode;
            priv->distm[n_chars].unicode2 = priv->characters[char_id].unicode2;
            ref1 = cursor;
            size1 = priv->characters[char_id].n_vectors;
            ref2 = ref1 + priv->characters[char_id].n_vectors * VEC_DIM_MAX;
            char_id++;

            priv->distm[n_chars+1].unicode = priv->characters[char_id].unicode;
            priv->distm[n_chars+1].unicode2 = priv->characters[char_id].unicode2;
            size2 = priv->characters[char_id].n_vectors;
            ref3 = ref2 + priv->characters[char_id].n_vectors * VEC_DIM_MAX;
            char_id++;

            priv->distm[n_chars+2].unicode = priv->characters[char_id].unicode;
            priv->distm[n_chars+2].unicode2 = priv->characters[char_id].unicode2;
            size3 = priv->characters[char_id].n_vectors;
            ref4 = ref3 + priv->characters[char_id].n_vectors * VEC_DIM_MAX;
            char_id++;

            priv->distm[n_chars+3].unicode = priv->characters[char_id].unicode;
            priv->distm[n_chars+3].unicode2 = priv->characters[char_id].unicode2;            
            size4 = priv->characters[char_id].n_vectors;
            cursor = ref4 + priv->characters[char_id].n_vectors *
                     VEC_DIM_MAX;
            char_id++;

            dtwres4 = dtw4(recognizer, input, n_vectors, 
                           ref1, size1, 
                           ref2, size2, 
                           ref3, size3, 
                           ref4, size4);

            priv->distm[n_chars++].dist = dtwres4.s[0];
            priv->distm[n_chars++].dist = dtwres4.s[1];
            priv->distm[n_chars++].dist = dtwres4.s[2];
            priv->distm[n_chars++].dist = dtwres4.s[3];
        }

        /* Process the remaining of references */
        n_group_chars = (priv->groups[group_id].n_chars % 4);
#else
        /* SSE not available, we need to process references sequentially */
        n_group_chars = priv->groups[group_id].n_chars;
#endif

        for (i=0; i < n_group_chars; i++) {
            priv->distm[n_chars].unicode = priv->characters[char_id].unicode;
            priv->distm[n_chars].unicode2 = priv->characters[char_id].unicode2;
            priv->distm[n_chars].dist = dtw(recognizer, input, n_vectors, 
                                      cursor, priv->characters[char_id].n_vectors);
            cursor += priv->characters[char_id].n_vectors * VEC_DIM_MAX;
            char_id++;
            n_chars++;
        }

    }

    /* sort the results with glibc's quicksort */
    qsort ((void *) priv->distm, 
           (size_t) n_chars, 
           sizeof (CharDist), 
           (int (*) (const void *, const void*)) char_dist_cmp);

    size = MIN(n_chars, n_results);

    WagomuResults *results = wagomu_results_new(size);

    for(i=0; i < size; i++)
        wagomu_results_add(results, i, priv->distm[i].unicode, priv->distm[i].unicode2, priv->distm[i].dist);
        //results->add(i, priv->distm[i].unicode, priv->distm[i].dist);

    return results;
}

char *  
wagomu_recognizer_get_error_message(WagomuRecognizer *recognizer)
{
    WagomuRecognizerPrivate *priv;

    priv = WAGOMU_RECOGNIZER_GET_PRIVATE(recognizer);
    g_return_val_if_fail (priv, NULL);
    return priv->error_msg;
}






WagomuResults*
wagomu_results_new(guint size)
{
    WagomuResults *r=g_new(WagomuResults, 1);    
    r->size = size;
    if (r->size > 0) {
        r->unicode = (unsigned int*) malloc(r->size * sizeof(unsigned int));
        r->unicode2 = (unsigned int*) malloc(r->size * sizeof(unsigned int));
        r->dist = (float *) malloc(r->size * sizeof(float));
    }
    return r;
}

WagomuResults*  
wagomu_results_copy(const WagomuResults *results)
{
    WagomuResults *new_results;

    g_return_val_if_fail (results, NULL);

    new_results = g_new (WagomuResults, 1);
    *new_results = *results;
    return new_results;
}

void
wagomu_results_free(WagomuResults *results)
{
    g_return_if_fail (results);
    if (results->size > 0) {
        if (results->unicode) free(results->unicode);
        if (results->unicode2) free(results->unicode2);
        if (results->dist) free(results->dist);
    }
    g_free (results);
}

void
wagomu_results_add(WagomuResults *results, guint i, guint unicode, guint unicode2, gfloat dist)
{
    g_return_if_fail (results);   
    results->unicode[i] = unicode;
    results->unicode2[i] = unicode2;
    results->dist[i] = dist;
}

guint
wagomu_results_get_unicode(WagomuResults *results, guint i, gboolean part)
{
    g_return_val_if_fail (results, 0);
    
    if(part)    
        return results->unicode2[i];
    return results->unicode[i];
}

gfloat
wagomu_results_get_distance(WagomuResults *results, guint i)
{
    g_return_val_if_fail (results, 0.0);
    return results->dist[i];
}

guint
wagomu_results_getsize(WagomuResults *results)
{
    g_return_val_if_fail (results, 0);
    return results->size;
}

GType
wagomu_results_get_type(void)
{
    static GType type = 0;

    if (type == 0) {
        const gchar *str;
#if GLIB_CHECK_VERSION(2, 10, 0)
        str = g_intern_static_string ("WagomuResults");
#else
        str = "WagomuResults";
#endif
        type =
            g_boxed_type_register_static (str,
                                          (GBoxedCopyFunc)wagomu_results_copy,
                                          (GBoxedFreeFunc)wagomu_results_free);
    }

    return type;
}

gfloat*
wagomu_character_get_points(WagomuCharacter *character)
{
    g_return_val_if_fail (character, NULL);
    return character->points;
}

guint
wagomu_character_get_n_vectors(WagomuCharacter *character)
{
    g_return_val_if_fail (character, 0);    
    return character->n_vectors;
}

guint
wagomu_character_get_n_strokes(WagomuCharacter *character)
{
    g_return_val_if_fail (character, 0);
    return character->n_strokes;
}


void
wagomu_character_set_value(WagomuCharacter *character, guint i, gfloat value)
{
    g_return_if_fail (character);
    character->points[i]=value;  
}

WagomuCharacter*
wagomu_character_copy(const WagomuCharacter *character)
{
     WagomuCharacter *new_character;

    g_return_val_if_fail (character, NULL);

    new_character = g_new (WagomuCharacter, 1);
    *new_character = *character;
    return new_character;
}

void
wagomu_character_free(WagomuCharacter *character)
{
    g_return_if_fail (character);
    if (character->n_vectors > 0)
        if (character->points) free(character->points);
    g_free (character);
}

GType
wagomu_character_get_type(void)
{
    static GType type = 0;

    if (type == 0) {
        const gchar *str;
#if GLIB_CHECK_VERSION(2, 10, 0)
        str = g_intern_static_string ("WagomuCharacter");
#else
        str = "WagomuCharacter";
#endif
        type =
            g_boxed_type_register_static (str,
                                          (GBoxedCopyFunc)wagomu_character_copy,
                                          (GBoxedFreeFunc)wagomu_character_free);
    }

    return type;
}



WagomuCharacter*
wagomu_character_new(guint n_vectors, guint n_strokes)
{
    WagomuCharacter *r=g_new(WagomuCharacter, 1);    
    r->n_vectors=n_vectors;
    r->n_strokes=n_strokes;
    if (n_vectors > 0)
        /*
        ptr = malloc(size+align+1);
        diff= ((-(int)ptr - 1)&(align-1)) + 1;
        ptr += diff;
        ((char*)ptr)[-1]= diff;
        */
        r->points = (float *) memalign(16, n_vectors * VEC_DIM_MAX *
                                       sizeof(float));
       // r->points = (float *) memalign(16, n_vectors * VEC_DIM_MAX *sizeof(float));
       // r->points = (float *) memalign(16, points*sizeof(float));
     //g_print("se separaron %d bloques para cada punto", n_vectors*VEC_DIM_MAX);
    return r;
}





/*
vi:ts=4:nowrap:ai:expandtab
*/
