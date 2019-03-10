/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */


#ifndef __MESSAGEFRAME_H__
#define __MESSAGEFRAME_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_MESSAGEFRAME            (message_frame_get_type ())
#define MESSAGEFRAME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MESSAGEFRAME, MessageFrame))
#define MESSAGEFRAME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MESSAGEFRAME, MessageFrame))
#define IS_MESSAGEFRAME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MESSAGEFRAME))
#define IS_MESSAGEFRAME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MESSAGEFRAME))
#define MESAGEFRAME_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_MESSAGEFRAME, MessageFrame))

typedef struct _MessageFrameClass MessageFrameClass;
typedef struct _MessageFrame MessageFrame;


struct _MessageFrame
{
    GtkFrame parent_instance;
};

struct _MessageFrameClass
{
    GtkFrameClass parent_class;

    /* -- signals -- */
    void (*s_continue)            (MessageFrame *mframe);
};


GType            message_frame_get_type             (void) G_GNUC_CONST;
MessageFrame     *message_frame_new                 (void);


/*
		***message_frame_set***
	gboolean corr: if this parameter is true it displays a correct! message otherwise incorrect! or similar message.
	const gchar* theans: if corr is false, it contains the original answer.
	const gchar* yourans: if corr is false, it contains the user answer.
	gboolean next: there is another question? in this case the button set its label text to "next", otherwise "finish"
	*/
	void message_frame_set(MessageFrame *mframe, gboolean corr, const gchar *theans, const gchar* yourans, gboolean next);



G_END_DECLS

#endif

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
