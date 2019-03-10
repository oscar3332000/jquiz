/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */


#ifndef __QUIZPAD_H__
#define __QUIZPAD_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define TYPE_QUIZPAD                (quiz_pad_get_type ())
#define QUIZPAD(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_QUIZPAD, QuizPad))
#define QUIZPAD_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_QUIZPAD, QuizPadClass))
#define IS_QUIZPAD(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_QUIZPAD))
#define IS_QUIZPAD_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_QUIZPAD))
#define QUIZPAD_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_QUIZPAD, QuizPadClass))


typedef struct _QuizPadClass QuizPadClass;
typedef struct _QuizPad QuizPad;

struct _QuizPad
{
    GtkFrame parent_instance;
};

struct _QuizPadClass
{
    GtkFrameClass parent_class;

    /* -- signals -- */
    void (*correct)            (QuizPad *qpad);
};

GType                   quiz_pad_get_type             (void) G_GNUC_CONST;

QuizPad                 *quiz_pad_new                 (void);

void                    quiz_pad_insert_kanji         (QuizPad *qpad,
                                                       gunichar kanji);

void                    quiz_pad_insert_clue           (QuizPad *qpad,
                                                       const gchar* pronun,
                                                       const gchar* meaning,
                                                       const gchar* answer);
void                    quiz_pad_clean_checks          (QuizPad *qpad);

const gchar *           quiz_pad_get_answer            (QuizPad *qpad);

/*void                    quiz_pad_set_answer             (QuizPad *qpad,
                                                         const gchar* ans);*/

void                    quiz_pad_replace_frame              (QuizPad *qpad, gboolean pad); //0 Quiz / 1 Training


G_END_DECLS
#endif

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
