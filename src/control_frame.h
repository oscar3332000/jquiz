/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef __CONTROLFRAME_H__
#define __CONTROLFRAME_H__

#include <gtk/gtk.h>
#include "groupnumber_dialog.h"


G_BEGIN_DECLS
#define TYPE_CONTROLFRAME            (control_frame_get_type ())
#define CONTROLFRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CONTROLFRAME, ControlFrame))
#define CONTROLFRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CONTROLFRAME, ControlFrameClass))
#define IS_CONTROLFRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CONTROLFRAME))
#define IS_CONTROLFRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CONTROLFRAME))
#define CONTROLFRAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CONTROLFRAME, ControlFrameClass))



typedef struct _ControlFrameClass ControlFrameClass;
typedef struct _ControlFrame ControlFrame;


struct _ControlFrame
{
    GtkFrame parent_instance;
};

struct _ControlFrameClass
{
    GtkFrameClass parent_class;

    /* -- signals -- */
    void (*open)            (ControlFrame *cframe);
    void (*quiz)            (ControlFrame *cframe);
    
    void (*quizm)           (ControlFrame *cframe);
    void (*trainm)          (ControlFrame *cframe);
};



GType               control_frame_get_type             (void) G_GNUC_CONST;

ControlFrame        *control_frame_new                 (void);

void                control_frame_set_quiz_sensitive   (ControlFrame* cframe,
                                                        gboolean sen);
gchar*              control_frame_get_filename         (ControlFrame *cframe);

void                control_frame_replace_frame        (ControlFrame *cframe, gboolean frame); //0 open, start / 1 quiz, training

gint                control_frame_get_gnumber           (ControlFrame *cframe);

gboolean            control_frame_get_order             (ControlFrame *cframe);



G_END_DECLS
#endif

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
