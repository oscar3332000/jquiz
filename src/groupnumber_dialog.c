/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */


#include "groupnumber_dialog.h"


typedef struct _GroupnumberDialogPriv GroupnumberDialogPriv;


struct _GroupnumberDialogPriv
{
    GtkWidget *number_entry;
    GtkWidget *order_check;

};


#define GROUPNUMBER_DIALOG_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_GROUPNUMBERDIALOG, GroupnumberDialogPriv))


static void                 groupnumber_dialog_finalize       (GObject            *object);
static void                 groupnumber_dialog_show           (GtkWidget          *widget);



G_DEFINE_TYPE (GroupnumberDialog, groupnumber_dialog, GTK_TYPE_DIALOG)



static void
groupnumber_dialog_class_init (GroupnumberDialogClass *klass)
{
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = (GObjectClass *)klass;
    widget_class = (GtkWidgetClass *)klass;


    object_class->finalize = groupnumber_dialog_finalize;

    widget_class->show = groupnumber_dialog_show;


    g_type_class_add_private (object_class, sizeof (GroupnumberDialogPriv));
}


static void
groupnumber_dialog_init (GroupnumberDialog *gndialog)
{

    GroupnumberDialogPriv  *priv=GROUPNUMBER_DIALOG_GET_PRIVATE(gndialog);

    GtkObject * adjust=gtk_adjustment_new (2,2, 10, 1,0,0);
    priv->number_entry=gtk_spin_button_new(GTK_ADJUSTMENT(adjust),1,0);
    priv->order_check=gtk_check_button_new_with_mnemonic("_in order");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG (gndialog)->vbox), GTK_WIDGET(priv->number_entry), FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(GTK_DIALOG (gndialog)->vbox), GTK_WIDGET(priv->order_check), FALSE, FALSE, 0);

    gtk_dialog_add_button  (GTK_DIALOG (gndialog),GTK_STOCK_OK,GTK_RESPONSE_OK);
    gtk_dialog_set_default_response (GTK_DIALOG (gndialog), GTK_RESPONSE_OK);

    gtk_window_set_title (GTK_WINDOW (gndialog), "training configuration");
    gtk_window_set_default_size (GTK_WINDOW (gndialog), 200, 50);
    //gtk_window_set_resizable (GTK_WINDOW (gndialog), FALSE);
    

    gtk_widget_show_all(GTK_WIDGET(gndialog));
    gtk_widget_hide (GTK_WIDGET (gndialog));

}

GtkWidget *
groupnumber_dialog_new (void)
{
  GroupnumberDialog *dialog = g_object_new (TYPE_GROUPNUMBERDIALOG, NULL);

  return GTK_WIDGET (dialog);
}

static void
groupnumber_dialog_finalize (GObject *object)
{
    G_OBJECT_CLASS (groupnumber_dialog_parent_class)->finalize (object);
}

static void
groupnumber_dialog_show (GtkWidget *widget)
{
  GTK_WIDGET_CLASS (groupnumber_dialog_parent_class)->show (widget);
}


void
show_groupnumber_dialog (GtkWindow   *parent)
{
  static GtkWidget *global_groupnumber_dialog = NULL;
  GtkWidget *dialog = NULL;

  if (parent)
    dialog = g_object_get_data (G_OBJECT (parent), "groupnumber-dialog");
  else
    dialog = global_groupnumber_dialog;

  if (!dialog)
    {
      dialog = groupnumber_dialog_new ();

      g_object_ref_sink (dialog);

      g_signal_connect (dialog, "delete-event",
                        G_CALLBACK (gtk_widget_hide_on_delete), NULL);

      /* Close dialog on user response */
      /*g_signal_connect (dialog, "response",
                        G_CALLBACK (close_cb), NULL);*/


      if (parent)
        {
          gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
          gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
          g_object_set_data_full (G_OBJECT (parent),
                                 "groupnumber-dialog",
                                  dialog, g_object_unref);
        }
      else
        global_groupnumber_dialog = dialog;

    }

  gtk_window_present (GTK_WINDOW (dialog));
}


gint
groupnumber_dialog_get_gnumber (GroupnumberDialog *gndialog)
{
    GroupnumberDialogPriv *priv = GROUPNUMBER_DIALOG_GET_PRIVATE(gndialog);
    g_return_val_if_fail(priv, -1);

    return (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(priv->number_entry));
}

gboolean
groupnumber_dialog_get_order (GroupnumberDialog *gndialog)
{
    GroupnumberDialogPriv *priv = GROUPNUMBER_DIALOG_GET_PRIVATE(gndialog);
    g_return_val_if_fail(priv, FALSE);
    GtkToggleButton *check = GTK_TOGGLE_BUTTON(priv->order_check);
    
    return gtk_toggle_button_get_active(check);
}



/*
 * vi:ts=4:nowrap:ai:expandtab
 */
