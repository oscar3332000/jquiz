/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 dialog to choose the number of items to study in the training mode
*/



#ifndef __GROUPNUMBERDIALOG_H__
#define __GROUPNUMBERDIALOG_H__

#include <gtk/gtk.h>


G_BEGIN_DECLS

#define TYPE_GROUPNUMBERDIALOG            (groupnumber_dialog_get_type ())
#define GROUPNUMBERDIALOG(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), TYPE_GROUPNUMBERDIALOG, GroupnumberDialog))
#define GROUPNUMBERDIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_GROUPNUMBERDIALOG, GroupnumberDialogClass))
#define IS_GROUPNUMBERDIALOG(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), TYPE_GROUPNUMBERDIALOG))
#define IS_GROUPNUMBERDIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_GROUPNUMBERDIALOG))
#define GROUPNUMBERDIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_GROUPNUMBERDIALOG, GroupnumberDialogClass))


typedef struct _GroupnumberDialog        GroupnumberDialog;
typedef struct _GroupnumberDialogClass   GroupnumberDialogClass;

struct _GroupnumberDialog 
{
  GtkDialog parent_instance;

  /*< private >*/
  gpointer GSEAL (private_data);
};

struct _GroupnumberDialogClass 
{
    GtkDialogClass parent_class;
     
};


GType               groupnumber_dialog_get_type             (void) G_GNUC_CONST;
GtkWidget          *groupnumber_dialog_new                  (void);
void                show_groupnumber_dialog                 (GtkWindow       *parent);
gint                groupnumber_dialog_get_gnumber          (GroupnumberDialog *gndialog);
gboolean            groupnumber_dialog_get_order            (GroupnumberDialog *gndialog);



G_END_DECLS
#endif

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
