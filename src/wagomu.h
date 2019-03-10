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
* converted to c by Oscar García
*/

#ifndef WAGOMU_H
#define WAGOMU_H

#include <glib-object.h>

#ifdef __SSE__
#include <xmmintrin.h>
#endif



G_BEGIN_DECLS

#define WAGOMU_TYPE_RECOGNIZER            (wagomu_recognizer_get_type ())
#define WAGOMU_RECOGNIZER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), WAGOMU_TYPE_RECOGNIZER, WagomuRecognizer))
#define WAGOMU_RECOGNIZER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), WAGOMU_TYPE_RECOGNIZER, WagomuRecognizerClass))
#define WAGOMU_IS_RECOGNIZER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WAGOMU_TYPE_RECOGNIZER))
#define WAGOMU_IS_RECOGNIZER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), WAGOMU_TYPE_RECOGNIZER))
#define WAGOMU_RECOGNIZER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WAGOMU_TYPE_RECOGNIZER, WagomuRecognizerClass))

#define WAGOMU_TYPE_CHARACTER             (character_get_type ())
#define WAGOMU_TYPE_RESULTS               (results_get_type ())

typedef struct _WagomuRecognizer WagomuRecognizer;
typedef struct _WagomuRecognizerClass WagomuRecognizerClass;
typedef struct _WagomuCharacter WagomuCharacter;
typedef struct _WagomuResults WagomuResults;
 

struct _WagomuRecognizer
{
    GObject object;
};

struct _WagomuRecognizerClass
{
    GObjectClass parent_class;
};


struct _WagomuResults
{
    guint *unicode;
    guint *unicode2;
    gfloat *dist;
    guint size;
};

struct _WagomuCharacter
{
    float *points;
    unsigned int n_vectors;
    unsigned int n_strokes;
};


#ifndef SWIG
typedef struct {
    unsigned int unicode;
    unsigned int unicode2;
    float dist;
} CharDist;

typedef struct {
    unsigned int unicode;
    unsigned int unicode2;
    unsigned int n_vectors;
} CharacterInfo;

typedef struct {
    unsigned int n_strokes;
    unsigned int n_chars;
    unsigned int offset;
    char pad[4];
} CharacterGroup;

#ifdef __SSE__
typedef union {
    __m128 v;
    float s[4];
} wg_v4sf;
#endif

#endif /* SWIG */

/* recognizer public functions*/

GType           wagomu_recognizer_get_type                          (void) G_GNUC_CONST;
WagomuRecognizer   *wagomu_recognizer_new                               (void);
gboolean        wagomu_recognizer_open                              (WagomuRecognizer *recognizer,
                                                                     char* path);
WagomuResults* wagomu_recognizer_recognize                         (WagomuRecognizer *recognizer, 
                                                                     WagomuCharacter *ch,
                                                                     guint n_results);
void            wagomu_recognizer_clear                             (WagomuRecognizer  *recognizer);

/*getters and setters*/
guint           wagomu_recognizer_get_n_characters                  (WagomuRecognizer *recognizer);
guint           wagomu_recognizer_get_dimension                     (WagomuRecognizer *recognizer);
guint           wagomu_recognizer_get_window_size                   (WagomuRecognizer *recognizer);
void            wagomu_recognizer_set_window_size                   (WagomuRecognizer *recognizer, 
                                                                     guint size);
char *          wagomu_recognizer_get_error_message                 (WagomuRecognizer *recognizer);

/*results public functions*/
GType           wagomu_results_get_type                             (void) G_GNUC_CONST;
WagomuResults*  wagomu_results_new                                  (guint size);
WagomuResults*  wagomu_results_copy                                 (const WagomuResults *results);
void            wagomu_results_free                                 (WagomuResults *results);
void            wagomu_results_add                                  (WagomuResults *results, 
                                                                     guint i,
                                                                     guint unicode,
                                                                     guint unicode2,
                                                                     gfloat dist);
guint           wagomu_results_get_unicode                          (WagomuResults *results,
                                                                     guint i,
                                                                     gboolean part);
gfloat          wagomu_results_get_distance                         (WagomuResults *results,
                                                                     guint i);
guint           wagomu_results_getsize                              (WagomuResults *results);

/*character public functions*/
GType                   wagomu_character_get_type                   (void) G_GNUC_CONST;
WagomuCharacter*        wagomu_character_new                        (guint n_vectors,
                                                                     guint n_strokes);
WagomuCharacter*        wagomu_character_copy                       (const WagomuCharacter *character);
void                    wagomu_character_free                       (WagomuCharacter *character);
gfloat*                 wagomu_character_get_points                 (WagomuCharacter *character);
guint                   wagomu_character_get_n_vectors              (WagomuCharacter *character);
guint                   wagomu_character_get_n_strokes              (WagomuCharacter *character);
void                    wagomu_character_set_value                  (WagomuCharacter *character,
                                                                     guint i,
                                                                     gfloat value);

G_END_DECLS
#endif /* __WAGOMU_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
