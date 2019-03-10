/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */


#include "control_frame.h"


enum {
    OPEN_SIGNAL,
    QUIZ_SIGNAL,
    QUIZ_MODE_SIGNAL,
    TRAINING_MODE_SIGNAL,
    LAST_SIGNAL
};

typedef struct _ControlFramePriv ControlFramePriv;

struct _ControlFramePriv
{
    GtkWidget *open; //open button
    GtkWidget *quiz; //quiz button
    GtkWidget *dialog; //open dialog
    GtkWidget *oqframe; //open /quiz frame container

    GtkWidget *quiz_mode; //quiz mode button
    GtkWidget *train_mode; //training mode button
    GtkWidget *gndialog; //group number dialog
    GtkWidget *qtframe; //quiz / trianing mode frame
    gchar *filename;
};

#define CONTROLFRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_CONTROLFRAME, ControlFramePriv))

G_DEFINE_TYPE (ControlFrame, control_frame, GTK_TYPE_FRAME)


static guint                control_signals[LAST_SIGNAL] = { 0 };


/* virtual functions for GtkObject class */
static void   control_frame_dispose          (GObject           *object);



static void control_frame_class_init(ControlFrameClass* klass)
{

    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
    //GtkFrameClass  *frame_class  = GTK_WIDGET_CLASS (klass);
    //GParamSpec *spec;

    gobject_class->dispose             = control_frame_dispose;
    
    control_signals[OPEN_SIGNAL]=g_signal_new ("open",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);

    control_signals[QUIZ_SIGNAL]=g_signal_new ("quiz",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);

    control_signals[QUIZ_MODE_SIGNAL]=g_signal_new ("quizm",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);

    control_signals[TRAINING_MODE_SIGNAL]=g_signal_new ("trainm",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);


    g_type_class_add_private (gobject_class, sizeof (ControlFramePriv));

}

void 
on_click_open_button(GtkWidget *widget, gpointer data)
{

    ControlFrame *cframe=CONTROLFRAME(data);
    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);


    //Show the dialog and wait for a user response:
    gint result = gtk_dialog_run(GTK_DIALOG(priv->dialog));
    gtk_widget_hide(priv->dialog);

    //Handle the response:

    if(result==GTK_RESPONSE_ACCEPT)
    {
        if(priv->filename!=NULL)
            g_free(priv->filename);
        
        priv->filename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(priv->dialog));
        g_signal_emit (G_OBJECT (data), control_signals[OPEN_SIGNAL], 0);
    }
    
	
}


void 
on_click_quiz_button(GtkWidget *widget, gpointer data)
{
    ControlFrame *cframe=CONTROLFRAME(data);
    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);

    if(priv->filename)
	    g_signal_emit(G_OBJECT(data), control_signals[QUIZ_SIGNAL],0);
}

void
on_click_quiz_mode_button(GtkWidget *widget, gpointer data)
{
    g_signal_emit(G_OBJECT(data), control_signals[QUIZ_MODE_SIGNAL],0);
}

void
on_click_training_mode_button(GtkWidget *widget, gpointer data)
{
    ControlFrame *cframe=CONTROLFRAME(data);
    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);

    gtk_dialog_run(GTK_DIALOG(priv->gndialog));
    gtk_widget_hide(priv->gndialog);

    g_signal_emit(G_OBJECT(data), control_signals[TRAINING_MODE_SIGNAL],0);
}

static void
control_frame_init(ControlFrame* cframe)
{

    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);


    

    priv->dialog=gtk_file_chooser_dialog_new ("Open File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

    priv->gndialog= groupnumber_dialog_new();

    //frame components

    GtkWidget *imop=gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    GtkWidget *imq=gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_MENU);
    GtkWidget *imqm=gtk_image_new_from_stock(GTK_STOCK_APPLY, GTK_ICON_SIZE_MENU);
    GtkWidget *imtm=gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU);
   
    GtkWidget *bbox=gtk_hbox_new(FALSE, 0);
    GtkWidget *albox=gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
    priv->oqframe=gtk_frame_new(NULL); 
    priv->open=gtk_button_new_with_mnemonic("_Open a file");
    priv->quiz=gtk_button_new_with_mnemonic("_Start a quiz");

    GtkWidget *bbox2=gtk_hbox_new(FALSE, 0);
    GtkWidget *albox2=gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
    priv->qtframe=gtk_frame_new(NULL); 
    priv->quiz_mode=gtk_button_new_with_mnemonic("_Quiz mode");
    priv->train_mode=gtk_button_new_with_mnemonic("_Training mode");

    gtk_button_set_image(GTK_BUTTON(priv->open), imop);
    gtk_button_set_image(GTK_BUTTON(priv->quiz), imq);
    gtk_widget_set_size_request(GTK_WIDGET(priv->open), 100, 100);
    gtk_widget_set_size_request(GTK_WIDGET(priv->quiz), 100, 100);
    gtk_signal_connect(GTK_OBJECT(priv->open), "clicked", GTK_SIGNAL_FUNC(on_click_open_button), cframe);
    gtk_signal_connect(GTK_OBJECT(priv->quiz), "clicked", GTK_SIGNAL_FUNC(on_click_quiz_button), cframe);
    gtk_box_pack_start(GTK_BOX(bbox), GTK_WIDGET(priv->open), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bbox), GTK_WIDGET(priv->quiz), FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(albox), bbox);
    gtk_container_add(GTK_CONTAINER(priv->oqframe), albox);


    gtk_button_set_image(GTK_BUTTON(priv->quiz_mode), imqm);
    gtk_button_set_image(GTK_BUTTON(priv->train_mode), imtm);
    gtk_widget_set_size_request(GTK_WIDGET(priv->quiz_mode), 100, 100);
    gtk_widget_set_size_request(GTK_WIDGET(priv->train_mode), 100, 100);
    gtk_signal_connect(GTK_OBJECT(priv->quiz_mode), "clicked", GTK_SIGNAL_FUNC(on_click_quiz_mode_button), cframe);
    gtk_signal_connect(GTK_OBJECT(priv->train_mode), "clicked", GTK_SIGNAL_FUNC(on_click_training_mode_button), cframe);
    gtk_box_pack_start(GTK_BOX(bbox2), GTK_WIDGET(priv->quiz_mode), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bbox2), GTK_WIDGET(priv->train_mode), FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(albox2), bbox2);
    gtk_container_add(GTK_CONTAINER(priv->qtframe), albox2);


   // gtk_container_add(GTK_CONTAINER(cframe), priv->oqframe);
   // gtk_widget_show_all(GTK_WIDGET(cframe));
    g_object_ref(priv->oqframe);
    g_object_ref(priv->qtframe);


}

static void
control_frame_dispose (GObject *object)
{
    //ControlFrame *cframe = CONTROLFRAME (object);
    //ControlFramePriv *priv = CONTROLFRAME_GET_PRIVATE(cframe);


    if(G_OBJECT_CLASS (control_frame_parent_class)->dispose)
        G_OBJECT_CLASS (control_frame_parent_class)->dispose (object);

    g_print("control-frame disposed\n");

}

ControlFrame *
control_frame_new (void)
{
    ControlFrame *cframe = g_object_new(TYPE_CONTROLFRAME, NULL);
    return cframe;
}

void 
control_frame_set_quiz_sensitive(ControlFrame* cframe, gboolean sen)
{
    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);

    g_return_if_fail(priv);

    gtk_widget_set_sensitive(GTK_WIDGET(priv->quiz), sen);
}

gchar*
control_frame_get_filename(ControlFrame *cframe)
{
    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);
    g_return_val_if_fail(priv, NULL);

    return priv->filename;
}


void 
control_frame_replace_frame (ControlFrame *cframe, gboolean frame)
{

    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);
    g_return_if_fail(priv);
    if(gtk_bin_get_child(GTK_BIN(cframe)))
        gtk_container_remove(GTK_CONTAINER(cframe), gtk_bin_get_child(GTK_BIN(cframe)));
	
    if(frame)
        gtk_container_add(GTK_CONTAINER(cframe), GTK_WIDGET(priv->qtframe));
    else
        gtk_container_add(GTK_CONTAINER(cframe), GTK_WIDGET(priv->oqframe));
    gtk_widget_show_all(GTK_WIDGET(cframe));
    g_print("cframe: cambio de cframe\n");

}


gint
control_frame_get_gnumber (ControlFrame *cframe)
{
    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);
    g_return_val_if_fail(priv, 2);
    return groupnumber_dialog_get_gnumber(GROUPNUMBERDIALOG(priv->gndialog));


}

gboolean
control_frame_get_order (ControlFrame *cframe)
{
    ControlFramePriv  *priv=CONTROLFRAME_GET_PRIVATE(cframe);
    g_return_val_if_fail(priv, FALSE);
    return groupnumber_dialog_get_order(GROUPNUMBERDIALOG(priv->gndialog));

}

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
