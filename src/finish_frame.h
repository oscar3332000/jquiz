/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef __FINISHFRAME_H__
#define __FINISHFRAME_H__


#include <gtk/gtk.h>


G_BEGIN_DECLS

#define TYPE_FINISHFRAME            (finish_frame_get_type ())
#define FINISHFRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_FINISHFRAME, FinishFrame))
#define FINISHFRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_FINISHFRAME, FinishFrameClass))
#define IS_FINISHFRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_FINISHFRAME))
#define IS_FINISHFRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_FINISHFRAME))
#define FINISHFRAME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_FINISHFRAME, FinishFrameClass))


typedef struct _FinishFrameClass FinishFrameClass;
typedef struct _FinishFrame FinishFrame;


struct _FinishFrame
{
    GtkFrame parent_instance;
};

struct _FinishFrameClass
{
    GtkFrameClass parent_class;

    /* -- signals -- */
    void (*finish)            (FinishFrame *fframe);
};



GType         finish_frame_get_type             (void) G_GNUC_CONST;

FinishFrame      *finish_frame_new                 (void);

/* ***setFinish***
	
	int c: correct questions
	int t: total questions
	
	*/
void            finish_frame_set                (FinishFrame* fframe, guint c,

                                              guint t);
G_END_DECLS
#endif
/*
 * vi:ts=4:nowrap:ai:expandtab
 */
