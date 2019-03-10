/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  2010 Oscar García
 *  This code is based on the work of Takuro Ashie(2005) and
 *  Juernjakob Harder(2006) from Tomoe project
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __TOMOE_CANVAS_H__
#define __TOMOE_CANVAS_H__

#include <gtk/gtk.h>
//#include <tomoe.h>
#include "tomoe-writing.h"

G_BEGIN_DECLS

#define TOMOE_CANVAS_DEFAULT_SIZE 200

#define TOMOE_TYPE_CANVAS            (tomoe_canvas_get_type ())
#define TOMOE_CANVAS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TOMOE_TYPE_CANVAS, TomoeCanvas))
#define TOMOE_CANVAS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TOMOE_TYPE_CANVAS, TomoeCanvasClass))
#define TOMOE_IS_CANVAS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TOMOE_TYPE_CANVAS))
#define TOMOE_IS_CANVAS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TOMOE_TYPE_CANVAS))
#define TOMOE_CANVAS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TOMOE_TYPE_CANVAS, TomoeCanvasClass))


typedef struct _TomoeCanvasClass TomoeCanvasClass;
typedef struct _TomoeCanvas      TomoeCanvas;

struct _TomoeCanvas
{
    GtkWidget   parent_instance;
};

struct _TomoeCanvasClass
{
    GtkWidgetClass parent_class;

    /* -- signals -- */
    void (*find)            (TomoeCanvas *canvas);
    void (*clear)           (TomoeCanvas *canvas);
    void (*normalize)       (TomoeCanvas *canvas);
    void (*stroke_added)    (TomoeCanvas *canvas);
    void (*stroke_reverted) (TomoeCanvas *canvas);
};


GType         tomoe_canvas_get_type             (void) G_GNUC_CONST;
GtkWidget    *tomoe_canvas_new                  (void);

/* setters/getters */
//void          tomoe_canvas_set_context          (TomoeCanvas  *canvas,
//                                                 TomoeContext *context);
void          tomoe_canvas_set_writing          (TomoeCanvas  *canvas,
                                                 TomoeWriting *writing);
TomoeWriting *tomoe_canvas_get_writing          (TomoeCanvas  *canvas);
void          tomoe_canvas_set_locked           (TomoeCanvas  *canvas,
                                                 gboolean      lock);
gboolean      tomoe_canvas_is_locked            (TomoeCanvas  *canvas);
void          tomoe_canvas_set_auto_find_time   (TomoeCanvas  *canvas,
                                                 gint          time_msec);
gint          tomoe_canvas_get_auto_find_time   (TomoeCanvas  *canvas);
void          tomoe_canvas_set_handwriting_line_color
                                                (TomoeCanvas  *canvas,
                                                 GdkColor     *color);
void          tomoe_canvas_set_adjusted_line_color
                                                (TomoeCanvas  *canvas,
                                                 GdkColor     *color);
void          tomoe_canvas_set_annotation_color (TomoeCanvas  *canvas,
                                                 GdkColor     *color);
void          tomoe_canvas_set_axis_color       (TomoeCanvas  *canvas,
                                                 GdkColor     *color);
void            tomoe_reduce                          (GtkWidget *widget);

/* editing/searching */
void          tomoe_canvas_find                 (TomoeCanvas  *canvas);
guint         tomoe_canvas_get_n_candidates     (TomoeCanvas  *canvas);
guint         tomoe_canvas_get_nth_candidate    (TomoeCanvas  *canvas,
                                                 guint         nth,
                                                 gboolean part);
//const WagomuResults  *tomoe_canvas_get_candidates       (TomoeCanvas  *canvas);
void          tomoe_canvas_revert_stroke        (TomoeCanvas  *canvas);
void          tomoe_canvas_clear                (TomoeCanvas  *canvas);
void          tomoe_canvas_normalize            (TomoeCanvas  *canvas);
gboolean      tomoe_canvas_has_stroke           (TomoeCanvas  *canvas);

G_END_DECLS

#endif /* __TOMOE_CANVAS_H__ */

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
