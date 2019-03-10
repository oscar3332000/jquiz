/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */




#include "finish_frame.h"

#define BAD "Train your vocabulary again"
#define GOOD "Good, train a little bit more"
#define EXCE "Excelent!, correct the mistakes"
#define PERF "Perfect!, pass to another lesson"


typedef struct _FinishFramePriv FinishFramePriv;

struct _FinishFramePriv
{
    guint correct;
	guint total;
	GtkWidget* result;
	GtkWidget* message;
	GtkWidget* bfinish;

};

#define FINISHFRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_FINISHFRAME, FinishFramePriv))

G_DEFINE_TYPE (FinishFrame, finish_frame, GTK_TYPE_FRAME)

static guint signal_value;
/* virtual functions for GtkObject class */
static void   finish_frame_dispose          (GObject           *object);


static void finish_frame_class_init(FinishFrameClass* klass)
{

    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
    //GtkFrameClass  *frame_class  = GTK_WIDGET_CLASS (klass);
    //GParamSpec *spec;

    gobject_class->dispose             = finish_frame_dispose;
    
    signal_value=g_signal_new ("finish",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);
    g_type_class_add_private (gobject_class, sizeof (FinishFramePriv));

}

void
on_click_finish(GtkWidget *widget, gpointer data)
{
    g_signal_emit (G_OBJECT (data), 
                    signal_value, 
                    0);
    
}

static void
finish_frame_init(FinishFrame* fframe)
{
    FinishFramePriv  *priv=FINISHFRAME_GET_PRIVATE(fframe);


    PangoFontDescription * FontFinish=pango_font_description_from_string("Sans 32");
    PangoFontDescription * FontResult=pango_font_description_from_string("Sans 64");
    PangoFontDescription * FontMessage=pango_font_description_from_string("Sans 12");

    GtkWidget *finish=gtk_label_new("Quiz finished");
    GtkWidget *mb = gtk_vbox_new(FALSE, 0);
    priv->message=gtk_label_new("");
    priv->result= gtk_label_new("");
    priv->bfinish= gtk_button_new_with_mnemonic("_continue");//gtk_button_new_with_label("continue"); 

    gtk_widget_modify_font(GTK_WIDGET(finish),FontFinish);
    gtk_widget_modify_font(GTK_WIDGET(priv->result),FontResult);
    gtk_widget_modify_font(GTK_WIDGET(priv->message),FontMessage);

    gtk_signal_connect(GTK_OBJECT(priv->bfinish), "clicked", GTK_SIGNAL_FUNC(on_click_finish), fframe);
    
    
    gtk_box_pack_start(GTK_BOX(mb), GTK_WIDGET(finish), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mb), GTK_WIDGET(priv->result), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mb), GTK_WIDGET(priv->message), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mb), GTK_WIDGET(priv->bfinish), FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(fframe), GTK_WIDGET(mb));
    gtk_widget_show_all(GTK_WIDGET(fframe));
    


    

}

static void
finish_frame_dispose (GObject *object)
{
    //FinishFrame *fframe = FINISHFRAME (object);
    //FinishFramePriv *priv = FINISHFRAME_GET_PRIVATE(fframe);


    if(G_OBJECT_CLASS (finish_frame_parent_class)->dispose)
    G_OBJECT_CLASS (finish_frame_parent_class)->dispose (object);

}


FinishFrame *
finish_frame_new (void)
{
    FinishFrame *fframe = g_object_new(TYPE_FINISHFRAME, NULL);
    return fframe;
}


void
finish_frame_set(FinishFrame* fframe, guint c, guint t)
{

    FinishFramePriv *priv = FINISHFRAME_GET_PRIVATE(fframe);

    int  val=(c*10)/t;
    GString *res;



	if(val<6)
		 gtk_label_set_text(GTK_LABEL(priv->message),BAD);
	else if(val < 8)
		gtk_label_set_text(GTK_LABEL(priv->message),GOOD);
	else if(val <10)
		gtk_label_set_text(GTK_LABEL(priv->message),EXCE);
	else
		gtk_label_set_text(GTK_LABEL(priv->message),PERF);
	
    res=g_string_new("");
    g_string_printf(res,"%d/%d", c, t);
    gtk_label_set_text(GTK_LABEL(priv->result), res->str);
}


/*
 * vi:ts=4:nowrap:ai:expandtab
 */
