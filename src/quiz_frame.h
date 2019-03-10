/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef __QUIZ_FRAME_H__
#define __QUIZ_FRAME_H__

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <glib.h>
#include <sqlite3.h>
#include "quizpad.h"
#include "message_frame.h"
#include "finish_frame.h"
#include "control_frame.h"

G_BEGIN_DECLS

#define TYPE_QUIZFRAME               (quiz_frame_get_type ())
#define QUIZFRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_QUIZFRAME, QuizFrame))
#define QUIZFRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_QUIZFRAME, QuizFrame))
#define IS_QUIZFRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_QUIZFRAME))
#define IS_QUIZFRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_QUIZFRAME))
#define QUIZFRAME_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_QUIZFRAME, QuizFrame))

#define TYPE_QUESTION              (question_get_type ())

typedef struct _QuizFrameClass QuizFrameClass;
typedef struct _QuizFrame QuizFrame;
typedef struct _Question Question; 

extern GMainLoop *loop;
extern GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
extern GstBus *bus;
extern void   on_pad_added (GstElement *element,
                            GstPad     *pad,
                            gpointer    data);

extern gboolean bus_call (GstBus     *bus,
                          GstMessage *msg,
                          gpointer    data);

struct _QuizFrame
{
    GtkWindow parent_instance;
};

struct _QuizFrameClass
{
    GtkWindowClass parent_class;
};

struct _Question{
	GString *id;
	GString *kanji;
	GString *pron;
	GString *meaning;
};

GType            quiz_frame_get_type             (void) G_GNUC_CONST;
QuizFrame     *quiz_frame_new                 (void);

GType           question_get_type            (void) G_GNUC_CONST;
Question     *question_new                   (void);
Question     *question_copy                (const Question *question);
void         question_free                (Question *question);
void         question_set                 (Question *question,
                                           const gchar *id,
                                           const gchar *kanji,
                                           const gchar *pron,
                                           const gchar *meaning);


G_END_DECLS

#endif


/*
 * vi:ts=4:nowrap:ai:expandtab
 */
