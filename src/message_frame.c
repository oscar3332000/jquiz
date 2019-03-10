/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */


#include "message_frame.h"


typedef struct _MessageFramePriv MessageFramePriv;


struct _MessageFramePriv
{
    GtkLabel *correct; //label
	GtkLabel *answer; //real answer //label
	GtkLabel *user; //user answer //label
    GtkLabel *etiq1;
    GtkLabel *etiq2;
	GtkButton *next;
	GtkHBox *ansbox; //this box contains
	GtkHBox *usbox;
};


#define MESSAGEFRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_MESSAGEFRAME, MessageFramePriv))

G_DEFINE_TYPE (MessageFrame, message_frame, GTK_TYPE_FRAME)

static guint signal_value;
/* virtual functions for GtkObject class */
static void   message_frame_dispose          (GObject           *object);


static void
message_frame_class_init(MessageFrameClass* klass)
{

    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
    //GtkFrameClass  *frame_class  = GTK_WIDGET_CLASS (klass);
    //GParamSpec *spec;

    gobject_class->dispose             = message_frame_dispose;
    
    signal_value=g_signal_new ("scontinue",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);
    g_type_class_add_private (gobject_class, sizeof (MessageFramePriv));

}


void
on_continue_clicked(GtkWidget *widget, gpointer data)
{
    g_signal_emit (G_OBJECT (data), 
                    signal_value, 
                    0);
}

static void
message_frame_init(MessageFrame* mframe)
{
    MessageFramePriv  *priv=MESSAGEFRAME_GET_PRIVATE(mframe);

    PangoFontDescription *FontAns=pango_font_description_from_string("serif 16");
    PangoFontDescription *FontUser=pango_font_description_from_string("serif 16");
    PangoFontDescription *BigFontEn=pango_font_description_from_string("sans 64");
 
    priv->etiq1=(GtkLabel *)gtk_label_new("The answer is:"); 
    priv->etiq2=(GtkLabel *)gtk_label_new("Your answer was:");   
    priv->correct=(GtkLabel *)gtk_label_new("");
    priv->answer=(GtkLabel *)gtk_label_new("");
    priv->user=(GtkLabel *)gtk_label_new("");
    priv->next=(GtkButton *)gtk_button_new_with_mnemonic("_Next");//gtk_button_new();

    GtkAlignment *alcorr=(GtkAlignment *)gtk_alignment_new(0.5, 1.0, 1.0,1.0);
    GtkAlignment *subox1=(GtkAlignment *)gtk_alignment_new(0.0, 1.0, 1.0,1.0);
    GtkAlignment *subox2=(GtkAlignment *)gtk_alignment_new(1.0, 1.0, 1.0,1.0);
    GtkAlignment *subox3=(GtkAlignment *)gtk_alignment_new(0.0, 1.0, 1.0,1.0);
    GtkAlignment *subox4=(GtkAlignment *)gtk_alignment_new(1.0, 1.0, 1.0,1.0);

    priv->ansbox=(GtkHBox*)gtk_hbox_new(TRUE, 0);
    priv->usbox=(GtkHBox*)gtk_hbox_new(TRUE, 0);
    GtkVBox *mbox=(GtkVBox*)gtk_vbox_new(FALSE, 8);

    gtk_widget_modify_font(GTK_WIDGET(priv->correct),BigFontEn);
    gtk_widget_modify_font(GTK_WIDGET(priv->answer),FontAns);
    gtk_widget_modify_font(GTK_WIDGET(priv->user),FontUser);

    gtk_label_set_selectable(priv->answer, TRUE);


    //first box
    gtk_container_add(GTK_CONTAINER(alcorr), GTK_WIDGET(priv->correct));
    
    //second box
    gtk_container_add(GTK_CONTAINER(subox1), GTK_WIDGET(priv->etiq1));
    gtk_container_add(GTK_CONTAINER(subox2), GTK_WIDGET(priv->answer));
    gtk_box_pack_start(GTK_BOX(priv->ansbox), GTK_WIDGET(subox1), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(priv->ansbox), GTK_WIDGET(subox2), FALSE, FALSE, 0);

    //third box
    gtk_container_add(GTK_CONTAINER(subox3), GTK_WIDGET(priv->etiq2));
    gtk_container_add(GTK_CONTAINER(subox4), GTK_WIDGET(priv->user));
    gtk_box_pack_start(GTK_BOX(priv->usbox), GTK_WIDGET(subox3), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(priv->usbox), GTK_WIDGET(subox4), FALSE, FALSE, 0);

    //fourth box
    gtk_signal_connect(GTK_OBJECT(priv->next), "clicked", GTK_SIGNAL_FUNC(on_continue_clicked), mframe);

    //main box
    gtk_box_pack_start(GTK_BOX(mbox), GTK_WIDGET(alcorr), TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mbox), GTK_WIDGET(priv->ansbox), TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mbox), GTK_WIDGET(priv->usbox), TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mbox), GTK_WIDGET(priv->next), TRUE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(mframe), GTK_WIDGET(mbox));
    gtk_widget_show_all(GTK_WIDGET(mframe));

}

static void
message_frame_dispose (GObject *object)
{
    //MessageFrame *mframe = MESSAGEFRAME (object);
    //MessageFramePriv *priv = MESSAGEFRAME_GET_PRIVATE(mframe);


    if(G_OBJECT_CLASS (message_frame_parent_class)->dispose)
        G_OBJECT_CLASS (message_frame_parent_class)->dispose (object);

}

MessageFrame *
message_frame_new (void)
{
    MessageFrame *mframe = g_object_new(TYPE_MESSAGEFRAME, NULL);
    return mframe;
}


void
message_frame_set(MessageFrame *mframe, gboolean corr, const gchar *theans, const gchar* yourans, gboolean next)
{

    MessageFramePriv *priv = MESSAGEFRAME_GET_PRIVATE(mframe);

	if(corr)
	{
        gtk_label_set_markup(priv->correct, "<span color=\"blue\" weight=\"bold\">Correct !</span>");
 
        gtk_label_set_text(priv->etiq1, "");
        gtk_label_set_text(priv->etiq2, ""); 
        gtk_label_set_text(priv->answer, "");
        gtk_label_set_text(priv->user, "");

        //doesn't work
        /*gtk_widget_hide(GTK_WIDGET(priv->etiq1));
        gtk_widget_hide(GTK_WIDGET(priv->etiq2));
        gtk_widget_hide(GTK_WIDGET(priv->answer));
        gtk_widget_hide(GTK_WIDGET(priv->user));
        gtk_widget_hide_all(GTK_WIDGET(priv->ansbox));
        gtk_widget_map(GTK_WIDGET(priv->ansbox));*/

	}
	else
	{
        gtk_label_set_markup(priv->correct, "<span color=\"red\" weight=\"bold\">Incorrect !</span>");
        gtk_label_set_text(priv->etiq1, "The answer is:");
        gtk_label_set_text(priv->etiq2, "Your answer was:"); 
        gtk_label_set_text(priv->answer, theans);
        gtk_label_set_text(priv->user, yourans);
        gtk_widget_show(GTK_WIDGET(priv->etiq1));
        gtk_widget_show(GTK_WIDGET(priv->etiq2));
        gtk_widget_show(GTK_WIDGET(priv->answer));
        gtk_widget_show(GTK_WIDGET(priv->user));
        gtk_widget_show_all(GTK_WIDGET(priv->ansbox));
	}

	if(next)
        gtk_button_set_label(priv->next, "_Next");
	else
		gtk_button_set_label(priv->next, "_Finish");
		
}


/*
 * vi:ts=4:nowrap:ai:expandtab
 */
