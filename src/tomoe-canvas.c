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

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <glib/gi18n.h>
#include <assert.h>

#include "tomoe-canvas.h"
#include "wagomu.h"

#define TOMOE_CANVAS_DEFAULT_RATE 0.7
#define TOMOE_CANVAS_MIN_NORMALIZE_SIZE 0.1
#define VECTOR_DIMENSION_MAX 4

enum {
    FIND_SIGNAL,
    FOUND_SIGNAL,
    CLEAR_SIGNAL,
    NORMALIZE_SIGNAL,
    STROKE_ADDED_SIGNAL,
    STROKE_REVERTED_SIGNAL,
    LAST_SIGNAL,
};

enum {
    PROP_0,
    //PROP_TOMOE_CONTEXT,
    PROP_LOCKED,
    PROP_WRITING,
    PROP_AUTO_FIND_TIME,
    PROP_HANDWRITING_LINE_COLOR,
    PROP_ADJUSTED_LINE_COLOR,
    PROP_ANNOTATION_COLOR,
    PROP_AXIS_COLOR
};

typedef struct _TomoeCanvasPriv TomoeCanvasPriv;
struct _TomoeCanvasPriv
{
    guint            size;
    gint             width;
    gint             height;

    GdkGC           *handwriting_line_gc;
    GdkGC           *adjusted_line_gc;
    GdkGC           *annotation_gc;
    GdkGC           *axis_gc;

    GdkPixmap       *pixmap;
    gboolean         drawing;

  //  TomoeContext    *context;
    TomoeWriting    *writing;
 //   GList           *candidates;

    gint             auto_find_time;
    guint            auto_find_id;
    gboolean         locked;
    WagomuRecognizer *recognizer;
    WagomuResults    *candidates;
    TomoePoint       *chk1;
    TomoePoint       *chk2;
    GThread          *fthread; //find thread   
};

#define TOMOE_CANVAS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TOMOE_TYPE_CANVAS, TomoeCanvasPriv))

G_DEFINE_TYPE (TomoeCanvas, tomoe_canvas, GTK_TYPE_WIDGET)

/* virtual functions for GtkObject class */
static void   dispose              (GObject           *object);
static void   set_property         (GObject           *object,
                                    guint              prop_id,
                                    const GValue      *value,
                                    GParamSpec        *pspec);
static void   get_property         (GObject           *object,
                                    guint              prop_id,
                                    GValue            *value,
                                    GParamSpec        *pspec);

/* virtual functions for GtkWidget class */
static void   realize              (GtkWidget         *widget);
static void   size_allocate        (GtkWidget         *widget,
                                    GtkAllocation     *allocation);
static gint   expose_event         (GtkWidget         *widget,
                                    GdkEventExpose    *event);
static gint   button_press_event   (GtkWidget         *widget,
                                    GdkEventButton    *event);
static gint   button_release_event (GtkWidget         *widget,
                                    GdkEventButton    *event);
static gint   motion_notify_event  (GtkWidget         *widget,
                                    GdkEventMotion    *event);

static void   tomoe_canvas_real_find            (TomoeCanvas       *canvas);
//static void   tomoe_canvas_thread_find          (TomoeCanvas       *canvas);
static void   tomoe_canvas_real_clear           (TomoeCanvas       *canvas);
static void   tomoe_canvas_real_normalize       (TomoeCanvas       *canvas);
static void   tomoe_canvas_append_point         (TomoeCanvas       *canvas,
                                                 gint               x,
                                                 gint               y);
static void   tomoe_canvas_draw_line            (TomoeCanvas       *canvas,
                                                 TomoePoint        *point1,
                                                 TomoePoint        *point2,
                                                 GdkGC             *line_gc,
                                                 gboolean           draw);
static void   tomoe_canvas_draw_background      (TomoeCanvas       *canvas,
                                                 gboolean           draw);
static void   tomoe_canvas_draw_axis            (TomoeCanvas       *canvas);
static void   tomoe_canvas_refresh              (TomoeCanvas       *canvas);
static void   tomoe_canvas_resize_writing       (TomoeCanvas       *canvas,
                                                 gdouble            x_rate,
                                                 gdouble            y_rate);
static void   tomoe_canvas_move_writing         (TomoeCanvas       *canvas,
                                                 gint               dx,
                                                 gint               dy);

static void   _init_gc                          (TomoeCanvas       *canvas);
static void   draw_stroke                       (GList             *points,
                                                 TomoeCanvas       *canvas,
                                                 guint              index);
static void   draw_annotation                   (GList             *points,
                                                 TomoeCanvas       *canvas,
                                                 guint              index);
//static void  reduce                             (TomoeCanvas *canvas);

static GList*               instance_list = NULL;
static guint                canvas_signals[LAST_SIGNAL] = { 0 };

#define _g_list_free_all(l, free_func) \
{\
    g_list_foreach (l, (GFunc) free_func, NULL); \
    g_list_free (l); \
}

static void
tomoe_canvas_class_init (TomoeCanvasClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class  = GTK_WIDGET_CLASS (klass);
    GParamSpec *spec;

    gobject_class->dispose             = dispose;
    gobject_class->set_property        = set_property;
    gobject_class->get_property        = get_property;
    widget_class->realize              = realize;
    widget_class->size_allocate        = size_allocate;
    widget_class->expose_event         = expose_event;
    widget_class->button_press_event   = button_press_event;
    widget_class->button_release_event = button_release_event;
    widget_class->motion_notify_event  = motion_notify_event;

    klass->find                        = tomoe_canvas_real_find;
    klass->clear                       = tomoe_canvas_real_clear;
    klass->normalize                   = tomoe_canvas_real_normalize;
    klass->stroke_added                = NULL;
    klass->stroke_reverted             = NULL;

    canvas_signals[FIND_SIGNAL] =
      g_signal_new ("find",
  		  G_TYPE_FROM_CLASS (klass),
  		  G_SIGNAL_RUN_LAST,
  		  G_STRUCT_OFFSET (TomoeCanvasClass, find),
  		  NULL, NULL,
  		  g_cclosure_marshal_VOID__VOID,
  		  G_TYPE_NONE, 0);
    canvas_signals[FOUND_SIGNAL] =
      g_signal_new ("found",
  		  G_TYPE_FROM_CLASS (klass),
  		  G_SIGNAL_RUN_LAST,
  		  0,
  		  NULL, NULL,
  		  g_cclosure_marshal_VOID__VOID,
  		  G_TYPE_NONE, 0);
    canvas_signals[CLEAR_SIGNAL] =
      g_signal_new ("clear",
  		  G_TYPE_FROM_CLASS (klass),
  		  G_SIGNAL_RUN_LAST,
  		  G_STRUCT_OFFSET (TomoeCanvasClass, clear),
  		  NULL, NULL,
  		  g_cclosure_marshal_VOID__VOID,
  		  G_TYPE_NONE, 0);
    canvas_signals[NORMALIZE_SIGNAL] =
      g_signal_new ("normalize",
  		  G_TYPE_FROM_CLASS (klass),
  		  G_SIGNAL_RUN_LAST,
 		  G_STRUCT_OFFSET (TomoeCanvasClass, normalize),
  		  NULL, NULL,
  		  g_cclosure_marshal_VOID__VOID,
  		  G_TYPE_NONE, 0);
    canvas_signals[STROKE_ADDED_SIGNAL] =
      g_signal_new ("stroke-added",
  		  G_TYPE_FROM_CLASS (klass),
  		  G_SIGNAL_RUN_LAST,
  		  G_STRUCT_OFFSET (TomoeCanvasClass, stroke_added),
  		  NULL, NULL,
  		  g_cclosure_marshal_VOID__VOID,
  		  G_TYPE_NONE, 0);
    canvas_signals[STROKE_REVERTED_SIGNAL] =
      g_signal_new ("stroke-reverted",
  		  G_TYPE_FROM_CLASS (klass),
  		  G_SIGNAL_RUN_LAST,
  		  G_STRUCT_OFFSET (TomoeCanvasClass, stroke_reverted),
  		  NULL, NULL,
  		  g_cclosure_marshal_VOID__VOID,
  		  G_TYPE_NONE, 0);

    /*spec = g_param_spec_object (
        "tomoe-context",
        N_("Tomoe context"),
        N_("A TomoeContext which stores handwriting dictionaries. "
           "TomoeCanvas doesn't always require TomoeContext. "
           "For example it isn't needed on viewer mode."),
        TOMOE_TYPE_CONTEXT,
        G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class, PROP_TOMOE_CONTEXT, spec);*/

    spec = g_param_spec_boolean (
        "locked",
        N_("Locked"),
        N_("Whether the canvas is locked drawing or not."),
        FALSE,
        G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class, PROP_LOCKED, spec);

    spec = g_param_spec_object (
        "writing",
        N_("Tomoe writing"),
        N_("Strokes of a character to show on this canvas."),
        TOMOE_TYPE_WRITING,
        G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class, PROP_WRITING, spec);

    spec = g_param_spec_int (
        "auto-find-time",
        N_("Auto find time"),
        N_("Delay time from releasing mouse button to starting auto finding. "
           "Set 0 to find immediatly. Set -1 to disable auto finding."),
        -1, G_MAXINT32, 0,
        G_PARAM_READABLE | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class, PROP_AUTO_FIND_TIME, spec);

    spec = g_param_spec_boxed (
        "handwriting-line-color",
        N_("Handwriting line color"),
        N_("The color of handwriting lines."),
        GDK_TYPE_COLOR,
        /*G_PARAM_READABLE |*/ G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_HANDWRITING_LINE_COLOR, spec);

    spec = g_param_spec_boxed (
        "adjusted-line-color",
        N_("Adjusted line color"),
        N_("The color of adjusted handwriting lines."),
        GDK_TYPE_COLOR,
        /*G_PARAM_READABLE |*/ G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_ADJUSTED_LINE_COLOR, spec);

    spec = g_param_spec_boxed (
        "annotation-color",
        N_("Annotation color"),
        N_("The color of annotation strings."),
        GDK_TYPE_COLOR,
        /*G_PARAM_READABLE |*/ G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_ANNOTATION_COLOR, spec);

    spec = g_param_spec_boxed (
        "axis-color",
        N_("Axis color"),
        N_("The color of axis lines."),
        GDK_TYPE_COLOR,
        /*G_PARAM_READABLE |*/ G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class, PROP_AXIS_COLOR, spec);

    g_type_class_add_private (gobject_class, sizeof(TomoeCanvasPriv));
}

static void
tomoe_canvas_init (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    gboolean wr=FALSE;
    
    priv->width               = TOMOE_CANVAS_DEFAULT_SIZE;
    priv->height              = TOMOE_CANVAS_DEFAULT_SIZE;

    priv->handwriting_line_gc = NULL;
    priv->adjusted_line_gc    = NULL;
    priv->annotation_gc       = NULL;
    priv->axis_gc             = NULL;

    priv->pixmap              = NULL;
    priv->drawing             = FALSE;

   // priv->context             = NULL;
    priv->writing             = tomoe_writing_new ();
    priv->candidates          = NULL;
    priv->recognizer          = wagomu_recognizer_new();


    priv->auto_find_time      = 0;
    priv->auto_find_id        = 0;
    priv->locked              = FALSE;
    
    priv->chk1                =tomoe_point_new(0, 0);
    priv->chk2                =tomoe_point_new(0, 0);

    instance_list = g_list_append (instance_list, (gpointer) canvas);
    wr=wagomu_recognizer_open(priv->recognizer,"wagomu/handwriting-ja.model");
    g_return_if_fail(wr);
}

static void
dispose (GObject *object)
{
    TomoeCanvas *canvas = TOMOE_CANVAS (object);
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    instance_list = g_list_remove (instance_list, (gpointer) canvas);

    if (priv->handwriting_line_gc) {
        g_object_unref (priv->handwriting_line_gc);
        priv->handwriting_line_gc = NULL;
    }

    if (priv->adjusted_line_gc) {
        g_object_unref (priv->adjusted_line_gc);
        priv->adjusted_line_gc = NULL;
    }

    if (priv->annotation_gc) {
        g_object_unref (priv->annotation_gc);
        priv->annotation_gc = NULL;
    }

    if (priv->axis_gc) {
        g_object_unref (priv->axis_gc);
        priv->axis_gc = NULL;
    }

    if (priv->pixmap) {
        g_object_unref (priv->pixmap);
        priv->pixmap = NULL;
    }

    if (priv->candidates) {
        //_g_list_free_all (priv->candidates, g_object_unref);
        wagomu_results_free(priv->candidates);
        priv->candidates     = NULL;
    }

    /*if (priv->context) {
        g_object_unref (priv->context);
        priv->context = NULL;
    }*/

    if (priv->writing) {
        g_object_unref (priv->writing);
        priv->writing = NULL;
    }
    
    if(priv->recognizer){
       g_object_unref (priv->recognizer);
       priv->recognizer=NULL;    
    }

    if (priv->auto_find_id) {
        g_source_remove (priv->auto_find_id);
        priv->auto_find_id = 0;
    }

    if (priv->chk1) {
        tomoe_point_free (priv->chk1);
        priv->chk1=NULL;
    }

    if (priv->chk2) {
        tomoe_point_free (priv->chk2);
        priv->chk2=NULL;
    }

    if (G_OBJECT_CLASS(tomoe_canvas_parent_class)->dispose)
        G_OBJECT_CLASS(tomoe_canvas_parent_class)->dispose(object);
}

static void
set_property (GObject *object,
              guint prop_id,
              const GValue *value,
              GParamSpec *pspec)
{
    TomoeCanvas *canvas = TOMOE_CANVAS (object);

    switch (prop_id) {
    /*case PROP_TOMOE_CONTEXT:
    {
        TomoeContext *ctx = TOMOE_CONTEXT (g_value_get_object (value));
        tomoe_canvas_set_context (canvas, ctx);
        break;
    }*/
    case PROP_LOCKED:
    {
        tomoe_canvas_set_locked (canvas, g_value_get_boolean (value));
        break;
    }
    case PROP_WRITING:
    {
        TomoeWriting *writing = TOMOE_WRITING (g_value_get_object (value));
        tomoe_canvas_set_writing (canvas, writing);
        break;
    }
    case PROP_AUTO_FIND_TIME:
    {
        tomoe_canvas_set_auto_find_time (canvas, g_value_get_int (value));
        break;
    }
    case PROP_HANDWRITING_LINE_COLOR:
    {
        GdkColor *color = g_value_get_boxed (value);
        tomoe_canvas_set_handwriting_line_color (canvas, color);
        break;
    }
    case PROP_ADJUSTED_LINE_COLOR:
    {
        GdkColor *color = g_value_get_boxed (value);
        tomoe_canvas_set_adjusted_line_color (canvas, color);
        break;
    }
    case PROP_ANNOTATION_COLOR:
    {
        GdkColor *color = g_value_get_boxed (value);
        tomoe_canvas_set_annotation_color (canvas, color);
        break;
    }
    case PROP_AXIS_COLOR:
    {
        GdkColor *color = g_value_get_boxed (value);
        tomoe_canvas_set_axis_color (canvas, color);
        break;
    }
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
get_property (GObject *object,
              guint prop_id,
              GValue *value,
              GParamSpec *pspec)
{
    TomoeCanvas *canvas = TOMOE_CANVAS (object);
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    switch (prop_id) {
    /*case PROP_TOMOE_CONTEXT:
        g_value_set_object (value, G_OBJECT (priv->context));
        break;*/
    case PROP_LOCKED:
    {
        g_value_set_boolean (value, priv->locked);
        break;
    }
    case PROP_WRITING:
    {
        TomoeWriting *writing = tomoe_canvas_get_writing (canvas);
        //g_value_set_object (value, G_OBJECT (priv->context));
        g_object_unref (writing);
        break;
    }
    case PROP_AUTO_FIND_TIME:
    {
        g_value_set_int (value, priv->auto_find_time);
        break;
    }
    case PROP_HANDWRITING_LINE_COLOR:
        /* FIXME */
        break;
    case PROP_ADJUSTED_LINE_COLOR:
        /* FIXME */
        break;
    case PROP_ANNOTATION_COLOR:
        /* FIXME */
        break;
    case PROP_AXIS_COLOR:
        /* FIXME */
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

GtkWidget *
tomoe_canvas_new (void)
{
    /*return GTK_WIDGET(g_object_new (TOMOE_TYPE_CANVAS,
                                    "tomoe-context", NULL,
                                    NULL));*/
    return GTK_WIDGET(g_object_new (TOMOE_TYPE_CANVAS,
                                    "", NULL,
                                    NULL));
}

static void
realize (GtkWidget *widget)
{
    PangoFontDescription *font_desc;
    GdkWindowAttr attributes;

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);

    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.event_mask = GDK_EXPOSURE_MASK |
	    GDK_BUTTON_PRESS_MASK |
	    GDK_BUTTON_RELEASE_MASK |
	    GDK_POINTER_MOTION_MASK |
	    GDK_POINTER_MOTION_HINT_MASK |
	    GDK_ENTER_NOTIFY_MASK |
	    GDK_LEAVE_NOTIFY_MASK;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                     &attributes,
                                     GDK_WA_X | GDK_WA_Y |
                                     GDK_WA_COLORMAP |
                                     GDK_WA_VISUAL);
    gdk_window_set_user_data (widget->window, widget);
    widget->style = gtk_style_attach (widget->style, widget->window);

    gdk_window_set_background (widget->window,
                               &widget->style->bg [GTK_STATE_NORMAL]);

    font_desc = pango_font_description_from_string ("Sans 12");
    gtk_widget_modify_font (widget, font_desc);
    pango_font_description_free (font_desc);
}


static void
size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (widget);

    if (GTK_WIDGET_CLASS (tomoe_canvas_parent_class)->size_allocate)
        GTK_WIDGET_CLASS (tomoe_canvas_parent_class)->size_allocate (widget, allocation);

    priv->width = allocation->width;
    priv->height = allocation->height;

    if (GTK_WIDGET_REALIZED (widget)) {
        if (priv->pixmap)
            g_object_unref(priv->pixmap);

        priv->pixmap = gdk_pixmap_new(widget->window,
                                      allocation->width,
                                      allocation->height,
                                      -1);
    	tomoe_canvas_refresh (TOMOE_CANVAS (widget));
    }
}

static gint
expose_event (GtkWidget *widget, GdkEventExpose *event)
{
    TomoeCanvas *canvas = TOMOE_CANVAS (widget);
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    gboolean retval = FALSE;

    if (GTK_WIDGET_CLASS(tomoe_canvas_parent_class)->expose_event)
        retval = GTK_WIDGET_CLASS(tomoe_canvas_parent_class)->expose_event (widget,
                                                                            event);
    gdk_draw_drawable(widget->window,
                      widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                      priv->pixmap,
                      event->area.x, event->area.y,
                      event->area.x, event->area.y,
                      event->area.width, event->area.height);

    return retval;
}

static gint
button_press_event (GtkWidget *widget, GdkEventButton *event)
{
    TomoeCanvas *canvas = TOMOE_CANVAS (widget);
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    gboolean retval = FALSE;

    if (priv->locked) return retval;

    if (priv->auto_find_id) {
        g_source_remove (priv->auto_find_id);
        priv->auto_find_id = 0;
    }

    if (event->button == 1) {
        priv->drawing = TRUE;
        tomoe_writing_move_to (priv->writing, (gint) event->x, (gint) event->y);
    }
    tomoe_canvas_refresh(canvas);
    return retval;
}

static gboolean
timeout_auto_find (gpointer user_data)
{
    tomoe_canvas_find (TOMOE_CANVAS (user_data));
    return FALSE;
}

static gint
button_release_event (GtkWidget *widget, GdkEventButton *event)
{
    TomoeCanvas *canvas = TOMOE_CANVAS (widget);
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    gboolean retval = FALSE;

    if (priv->locked) return retval;
    if (!priv->drawing) return retval;

    /* FIXME: draw annotation */

    priv->drawing = FALSE;

    g_signal_emit (G_OBJECT (widget), canvas_signals[STROKE_ADDED_SIGNAL], 0);

    if (priv->auto_find_id) {
        g_source_remove (priv->auto_find_id);
        priv->auto_find_id = 0;
    }
    if (priv->auto_find_time >= 0) {
        priv->auto_find_id = g_timeout_add (priv->auto_find_time,
                                            timeout_auto_find,
                                            (gpointer)canvas);
    }

    return retval;
}

static gint
motion_notify_event (GtkWidget *widget, GdkEventMotion *event)
{
    TomoeCanvas *canvas = TOMOE_CANVAS (widget);
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    gint x, y;
    GdkModifierType state;
    gboolean retval = FALSE;

    if (priv->locked) return retval;
    if (!priv->drawing) return retval;

    if (event->is_hint) {
        gdk_window_get_pointer(event->window, &x, &y, &state);
    } else {
        x = (gint) event->x;
        y = (gint) event->y;
        state = (GdkModifierType) event->state;
    }
    //check if the distance worths!
    //    chk1->x
    tomoe_canvas_append_point (canvas, x, y);

    return retval;
}

inline static float
tomoe_point_euclidean_distance(TomoePoint *u, TomoePoint *v)
{   
    /*g_print("\npunto prevx:%d\n", u->x);
    g_print("\npunto despx:%d\n", v->x);
    g_print("\npunto prevy:%d\n", u->y);
    g_print("\npunto despy:%d\n", v->y);
    g_print("\nabsx:%d, absy:%d, powx:%f,  powy:%f\n", ABS(v->x-u->x), ABS(v->y-u->y), pow((int)ABS(v->x-u->x),2), pow((int)ABS(v->y-u->y),2));
    g_print("\nabsy:%d\n", ABS(v->y-u->y));
    g_print("\ndistancia euclideana %f\n", sqrt(pow(ABS(v->x-u->x), 2)+ pow(ABS(v->y-u->y),2)));*/
    return sqrt(pow(ABS(v->x-u->x), 2)+ pow(ABS(v->y-u->y),2));
}

static TomoeWriting*
_tomoe_writing_downsample_threshold(TomoeWriting *writing)
{
    const GList *strokes, *list;
    TomoeWriting *new = tomoe_writing_new ();

    strokes = tomoe_writing_get_strokes (writing);
    for (list= strokes; list; list = g_list_next (list)) {
        GList *points = (GList *) list->data;
        GList *point;
        gboolean first = TRUE;
        TomoePoint *cp=NULL;
        TomoePoint *cp2=NULL;
        
        g_print("\ninicio euclides\n");
        
        for ( point = points; point; point = g_list_next (point)) {

            if(!cp)            
            cp = (TomoePoint *) point->data;            

            
            if(g_list_next(point))
                cp2=(TomoePoint *)g_list_next(point)->data;
            else
                continue;

            if(first)
                tomoe_writing_move_to (new, cp->x, cp->y);

            g_print("\ndistancia euclideana:%f\n",tomoe_point_euclidean_distance(cp, cp2));
            if(tomoe_point_euclidean_distance(cp, cp2)>TOMOE_WRITING_THRESHOLD)
            {
                tomoe_writing_line_to (new, cp2->x, cp2->y);
                cp=cp2;
            }

            first = FALSE;
        }
    }
    return new;
}

static guint
_tomoe_writing_get_n_points(TomoeWriting* writing)
{    
    const GList *list=tomoe_writing_get_strokes(writing);
    GList *stroke;
    guint points=0;

    g_return_val_if_fail(writing,0);

    //for(stroke=list->data; stroke; stroke=g_list_next (stroke))
        //points+=g_list_length(stroke);
    for(;list;list=g_list_next (list))
    {
        stroke=list->data;
        points+=g_list_length(stroke);
    }
    return points;
    
}

static GList *
_tomoe_writing_get_delta_values(TomoeWriting* writing)
{
    const GList *strokes, *list;
    //TomoeWriting *new = tomoe_writing_new ();
    GList *new=NULL, *last=NULL;
    TomoePoint *cp=NULL;
    TomoePoint *cp2=NULL;
    strokes = tomoe_writing_get_strokes (writing);
    g_print("puntos:%d\n", _tomoe_writing_get_n_points(writing));
    for (list= strokes; list; list = g_list_next (list)) {
        GList *points = (GList *) list->data;
        GList *point;
       // gboolean first = TRUE;
        

        
        for ( point = points; point; point = g_list_next (point)) {
            
            if(!cp)
            {    
                cp = (TomoePoint *) point->data;
                continue;            
            }
            else
               cp2 = (TomoePoint *)point->data;  

            /*if(first)
                tomoe_writing_move_to (new, ABS(cp2->x-cp->x), ABS(cp2->y-cp->y));

            else
                tomoe_writing_line_to (new, ABS(cp2->x-cp->x), ABS(cp2->y-cp->y));*/

          //  first = FALSE;
          last=g_list_append (last, GINT_TO_POINTER (ABS(cp2->x-cp->x)));
          if(!new)
            new=last;
          last=g_list_last (last);
          last=g_list_append (last, GINT_TO_POINTER (ABS(cp2->y-cp->y)));
          last=g_list_last (last);
          last=g_list_append (last, GINT_TO_POINTER (0));
          last=g_list_last (last);
          last=g_list_append (last, GINT_TO_POINTER (0));
          last=g_list_last (last);
          cp=cp2;
        }

    }
    //g_print("inicial: %d",tomoe_writing_get_n_strokes(writing));
    //g_print("final: %d",tomoe_writing_get_n_strokes(new));
    //assert(tomoe_writing_get_n_strokes(new)==tomoe_writing_get_n_strokes(writing));
    return new;


    /*GList *lista=NULL;
    GList *last=NULL;
    const GList *strokes, *stroke, *points;
    TomoePoint *p;

    g_print("\npuntos:%d\n", _tomoe_writing_get_n_points(writing));
    strokes=tomoe_writing_get_strokes (writing);
    for(; strokes; strokes=g_list_next (strokes))
    {
        stroke=strokes->data;        
        for(; stroke;stroke=g_list_next (stroke))
        {
            p=stroke->data;
            last=g_list_append (last, p);
            if(!lista)
                lista=last;
            last=g_list_last (last);
            
        }
    }
    return lista;*/

}

static TomoeWriting *
_tomoe_writing_new_scale_writing (TomoeWriting *writing, gdouble sx, gdouble sy)
{
    const GList *strokes, *list;
    TomoeWriting *new = tomoe_writing_new ();

    strokes = tomoe_writing_get_strokes (writing);
    for (list= strokes; list; list = g_list_next (list)) {
        GList *points = (GList *) list->data;
        GList *point;
        gboolean first = TRUE;
        for ( point = points; point; point = g_list_next (point)) {
            TomoePoint *cp = (TomoePoint *) point->data;
            if (!first)
                tomoe_writing_line_to (new, cp->x * sx, cp->y * sy);
            else
                tomoe_writing_move_to (new, cp->x * sx, cp->y * sy);
            first = FALSE;
        }
    }
    return new;
}

/*static void tomoe_canvas_thread_find(TomoeCanvas *canvas)
{
    GError *err = NULL; 
    if( g_thread_create((GThreadFunc)tomoe_canvas_real_find, canvas, FALSE, &err) == NULL)
	{
	     g_print("Thread create failed: %s!!\n", err->message );
	     g_error_free ( err ) ;
	}
}*/

static void
tomoe_canvas_real_find (TomoeCanvas *canvas)
{
    GtkWidget *widget = GTK_WIDGET (canvas);
    TomoeCanvasPriv *priv;
    TomoeWriting *writing, *w;
    WagomuCharacter *ch;
    const GList *strokes; 
    const GList *list;
    GList *lista;
    guint i, nvectors,n_results;
    n_results=5;

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    if (tomoe_writing_get_n_strokes (priv->writing) == 0)
        return;

    _init_gc (canvas);

    /* draw thin red lines and annotations for sparse points */
    strokes = tomoe_writing_get_strokes (priv->writing);
    for (list = strokes, i = 1; list; list = g_list_next (list), i++) {
        GList *points = (GList *) list->data;
 
        draw_annotation (points, canvas, i);
    }

    if (priv->candidates) {
        //_g_list_free_all (priv->candidates, g_object_unref);
        wagomu_results_free(priv->candidates);
        priv->candidates     = NULL;
    }
    
    writing = _tomoe_writing_new_scale_writing (
        priv->writing,
        (gdouble) TOMOE_WRITING_WIDTH  / priv->width,
        (gdouble) TOMOE_WRITING_HEIGHT / priv->height);

    w=_tomoe_writing_downsample_threshold(writing);
    g_object_unref (writing);
    writing=w;
    nvectors=_tomoe_writing_get_n_points(writing)/VECTOR_DIMENSION_MAX;
    lista=_tomoe_writing_get_delta_values(writing);
    nvectors=g_list_length(lista)/VECTOR_DIMENSION_MAX;
    ch=wagomu_character_new(nvectors, tomoe_writing_get_n_strokes(writing));
    for(i=0;lista;lista=g_list_next (lista), i++)
        wagomu_character_set_value(ch, i,(float)GPOINTER_TO_INT(lista->data));
    
    g_list_free(lista);

    priv->candidates=wagomu_recognizer_recognize(priv->recognizer,ch,n_results);
   
    
    g_object_unref (writing);
    gdk_draw_drawable(widget->window,
                      widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                      priv->pixmap,
                      0, 0,
                      0, 0,
                      widget->allocation.width, widget->allocation.height);
    //gdk_threads_enter();
    g_signal_emit (G_OBJECT (canvas), canvas_signals[FOUND_SIGNAL], 0);
    //gdk_threads_leave();
}

static void
tomoe_canvas_real_clear (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv;

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    _init_gc (canvas);
    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    tomoe_canvas_draw_background (canvas, TRUE);

    if (priv->candidates) {
        //_g_list_free_all (priv->candidates, g_object_unref);
        wagomu_results_free(priv->candidates);
        priv->candidates     = NULL;
    }

    tomoe_writing_clear (priv->writing);

    tomoe_canvas_refresh (canvas);
}

/*
void
tomoe_canvas_set_context (TomoeCanvas *canvas, TomoeContext *context)
{
    TomoeCanvasPriv *priv;

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    if (priv->context)
        g_object_unref (priv->context);
    if (context)
        g_object_ref (context);
    priv->context = context;

    g_object_notify (G_OBJECT (canvas), "tomoe-context");
}
*/

void
tomoe_canvas_set_locked (TomoeCanvas *canvas, gboolean lock)
{
    TomoeCanvasPriv *priv;

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    priv->locked = lock;

    g_object_notify (G_OBJECT (canvas), "locked");
}

gboolean
tomoe_canvas_is_locked (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv;

    g_return_val_if_fail (TOMOE_IS_CANVAS (canvas), FALSE);

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    return priv->locked;
}

void
tomoe_canvas_set_writing (TomoeCanvas *canvas, TomoeWriting *writing)
{
    TomoeCanvasPriv *priv;
    TomoeWriting *new_writing = NULL;

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    if (writing)
        new_writing = _tomoe_writing_new_scale_writing (
            writing,
            (gdouble) priv->width  / TOMOE_WRITING_WIDTH,
            (gdouble) priv->height / TOMOE_WRITING_HEIGHT);

    if (priv->writing)
        g_object_unref (priv->writing);
    priv->writing = new_writing;

    if (GTK_WIDGET_REALIZED (GTK_WIDGET (canvas)))
    	tomoe_canvas_refresh (canvas);

    g_object_notify (G_OBJECT (canvas), "writing");
}

TomoeWriting *
tomoe_canvas_get_writing (TomoeCanvas *canvas)
{
    TomoeWriting *writing = NULL;
    TomoeCanvasPriv *priv;

    g_return_val_if_fail (TOMOE_IS_CANVAS (canvas), NULL);

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    if (priv->writing)
        writing = _tomoe_writing_new_scale_writing (
            priv->writing,
            (gdouble) TOMOE_WRITING_WIDTH  / priv->width,
            (gdouble) TOMOE_WRITING_HEIGHT / priv->height);

    return writing;
}

void
tomoe_canvas_set_auto_find_time (TomoeCanvas *canvas, gint time_msec)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    if (time_msec < 0)
        priv->auto_find_time = -1;
    else
        priv->auto_find_time = time_msec;

    g_object_notify (G_OBJECT (canvas), "auto-find-time");
}

gint
tomoe_canvas_get_auto_find_time (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    g_return_val_if_fail (TOMOE_IS_CANVAS (canvas), -1);

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    return priv->auto_find_time;
}

static void
tomoe_canvas_gc_set_foreground (GdkGC *gc, GdkColor *color)
{
    GdkColor default_color = { 0, 0x0000, 0x0000, 0x0000 };

    if (color) {
        gdk_colormap_alloc_color (gdk_colormap_get_system (), color,
                                  TRUE, TRUE);
        gdk_gc_set_foreground (gc, color);
    } else {
        gdk_colormap_alloc_color (gdk_colormap_get_system (), &default_color,
                                  TRUE, TRUE);
        gdk_gc_set_foreground (gc, &default_color);
    }
}

void
tomoe_canvas_set_handwriting_line_color (TomoeCanvas *canvas, GdkColor *color)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    tomoe_canvas_gc_set_foreground (priv->handwriting_line_gc, color);

    g_object_notify (G_OBJECT (canvas), "handwriting-line-color");
}

void
tomoe_canvas_set_adjusted_line_color (TomoeCanvas *canvas, GdkColor *color)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    tomoe_canvas_gc_set_foreground (priv->adjusted_line_gc, color);

    g_object_notify (G_OBJECT (canvas), "adjusted-line-color");
}

void
tomoe_canvas_set_annotation_color (TomoeCanvas *canvas, GdkColor *color)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    tomoe_canvas_gc_set_foreground (priv->annotation_gc, color);

    g_object_notify (G_OBJECT (canvas), "annotation-color");
}

void
tomoe_canvas_set_axis_color (TomoeCanvas *canvas, GdkColor *color)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    tomoe_canvas_gc_set_foreground (priv->axis_gc, color);

    g_object_notify (G_OBJECT (canvas), "axis-color");
}

static void
get_rectangle_for_stroke (GList *points, GdkRectangle *rect)
{
    GList *list;

    for (list = points; list; list = g_list_next (list)) {
        gint x = ((TomoePoint*)(list->data))->x;
        gint y = ((TomoePoint*)(list->data))->y;

        rect->x = MIN (rect->x, x);
        rect->y = MIN (rect->y, y);
        rect->width  = MAX (rect->width, x - rect->x);
        rect->height = MAX (rect->height, y - rect->y);
    }
}

static void
get_char_size (TomoeCanvas *canvas, GdkRectangle *rect)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    const GList *strokes, *list;
    TomoeWriting *g = priv->writing;

    rect->x = G_MAXINT;
    rect->y = G_MAXINT;
    rect->width = 0;
    rect->height = 0;

    strokes = tomoe_writing_get_strokes (g);
    for (list= strokes; list; list = g_list_next (list)) {
        GList *points = (GList *) list->data;
        GList *point;
        for (point = points; point; point = g_list_next (point)) {
            get_rectangle_for_stroke (points, rect);
        }
    }
}

static void
tomoe_canvas_resize_writing (TomoeCanvas *canvas,
                             gdouble x_rate, gdouble y_rate)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    TomoeWriting *new_writing;

    new_writing = _tomoe_writing_new_scale_writing (priv->writing,
                                                    x_rate, y_rate);

    if (priv->writing)
        g_object_unref (priv->writing);
    priv->writing = new_writing;

    if (GTK_WIDGET_REALIZED (GTK_WIDGET (canvas)))
    	tomoe_canvas_refresh (canvas);
}

static TomoeWriting *
_tomoe_writing_new_move_writing (TomoeWriting *writing, gint dx, gint dy)
{
    const GList *strokes, *list;
    TomoeWriting *new = tomoe_writing_new ();

    strokes = tomoe_writing_get_strokes (writing);
    for (list= strokes; list; list = g_list_next (list)) {
        GList *points = (GList *) list->data;
        GList *point;
        gboolean first = TRUE;
        for ( point = points; point; point = g_list_next (point)) {
            TomoePoint *cp = (TomoePoint *) point->data;
            if (!first)
                tomoe_writing_line_to (new, cp->x + dx, cp->y + dy);
            else
                tomoe_writing_move_to (new, cp->x + dx, cp->y + dy);
            first = FALSE;
        }
    }
    return new;
}

static void
tomoe_canvas_move_writing (TomoeCanvas *canvas, gint dx, gint dy)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    TomoeWriting *new_writing;

    new_writing = _tomoe_writing_new_move_writing (priv->writing, dx, dy);

    if (priv->writing)
        g_object_unref (priv->writing);
    priv->writing = new_writing;

    if (GTK_WIDGET_REALIZED (GTK_WIDGET (canvas)))
    	tomoe_canvas_refresh (canvas);
}

static void
tomoe_canvas_real_normalize (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    GdkRectangle char_size;
    gdouble x_rate, y_rate;
    gint dx, dy;

    get_char_size (canvas, &char_size);

    if(char_size.width/priv->width>TOMOE_CANVAS_MIN_NORMALIZE_SIZE)
        x_rate = (priv->width  * TOMOE_CANVAS_DEFAULT_RATE) / char_size.width;
    else
        x_rate=1.0;
    if(char_size.height/priv->height>TOMOE_CANVAS_MIN_NORMALIZE_SIZE)
        y_rate = (priv->height * TOMOE_CANVAS_DEFAULT_RATE) / char_size.height;
    else
        y_rate=1.0;

    tomoe_canvas_resize_writing (canvas, x_rate, y_rate);

    get_char_size (canvas, &char_size);
    dx = ((priv->width  - char_size.width)  / 2) - char_size.x;
    dy = ((priv->height - char_size.height) / 2) - char_size.y;

    tomoe_canvas_move_writing (canvas, dx, dy);

    tomoe_canvas_refresh (canvas);
    tomoe_canvas_find (canvas);
}

void
tomoe_canvas_find (TomoeCanvas *canvas)
{
    g_return_if_fail (TOMOE_IS_CANVAS (canvas));
    g_signal_emit (G_OBJECT (canvas), canvas_signals[FIND_SIGNAL], 0);
}

guint
tomoe_canvas_get_n_candidates (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv;

    g_return_val_if_fail (TOMOE_IS_CANVAS (canvas), 0);

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    return wagomu_results_getsize(priv->candidates);//g_list_length (priv->candidates);
}

guint
tomoe_canvas_get_nth_candidate(TomoeCanvas  *canvas, guint nth, gboolean part)
{
    TomoeCanvasPriv *priv;

    g_return_val_if_fail (TOMOE_IS_CANVAS (canvas), 0);

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    if (nth < wagomu_results_getsize(priv->candidates)) {
 
        return wagomu_results_get_unicode(priv->candidates, nth, part);
    } else {
        return 0;
    }
}

/*const WagomuResults *
tomoe_canvas_get_candidates (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    return priv->candidates;
}*/

void
tomoe_canvas_revert_stroke (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv;
    gint n;

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    n = tomoe_writing_get_n_strokes (priv->writing);
    if (n <= 0) return;

    /* remove last line */
    tomoe_writing_remove_last_stroke (priv->writing);
    tomoe_canvas_refresh (canvas);

    /* emit signal */
    g_signal_emit (G_OBJECT (canvas),
                   canvas_signals[STROKE_REVERTED_SIGNAL], 0);
    if (tomoe_writing_get_n_strokes (priv->writing) == 0)
        g_signal_emit (G_OBJECT (canvas), canvas_signals[CLEAR_SIGNAL], 0);

}

void
tomoe_canvas_clear (TomoeCanvas *canvas)
{
    g_return_if_fail (TOMOE_IS_CANVAS (canvas));
    g_signal_emit (G_OBJECT (canvas), canvas_signals[CLEAR_SIGNAL], 0);
}

void
tomoe_canvas_normalize (TomoeCanvas *canvas)
{
    g_return_if_fail (TOMOE_IS_CANVAS (canvas));
    g_signal_emit (G_OBJECT (canvas), canvas_signals[NORMALIZE_SIGNAL], 0);
}

gboolean
tomoe_canvas_has_stroke (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);

    g_return_val_if_fail (TOMOE_IS_CANVAS (canvas), FALSE);

    return (tomoe_writing_get_n_strokes (priv->writing) > 0);
}

static void
tomoe_canvas_append_point (TomoeCanvas *canvas, gint x, gint y)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    TomoePoint pp, p;//, ppp, p1;
    GList *strokes;
    GList *point = NULL;

    p.x = x;
    p.y = y;
    
    //p1.x=x*(1000/300);
    //p1.y=y*(1000/300);

    strokes = (GList *) tomoe_writing_get_strokes (priv->writing);
    strokes = g_list_last (strokes);
    g_return_if_fail (strokes);

    point = strokes->data;
    g_return_if_fail (point);

    point = g_list_last (point);
    g_return_if_fail (point->data);
    pp = *((TomoePoint*) point->data);

    //ppp.x=pp.x*(1000/300);
    //ppp.y=pp.y*(1000/300);

    
        
        
        _init_gc (canvas);
        tomoe_canvas_draw_line (canvas, &pp, &p,
                                priv->handwriting_line_gc,
                                TRUE);
    //if(tomoe_point_euclidean_distance(&ppp, &p1)>TOMOE_WRITING_THRESHOLD)
    //{
      //  g_printf("pasó porque: %f\n",tomoe_point_euclidean_distance(&ppp, &p1));
        tomoe_writing_line_to (priv->writing, x, y);
    //}
}

static void
tomoe_canvas_draw_line (TomoeCanvas *canvas,
                        TomoePoint *p1, TomoePoint *p2,
                        GdkGC *line_gc,
                        gboolean draw)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    GtkWidget *widget = GTK_WIDGET (canvas);
    gint x, y, width, height;

    gdk_draw_line (priv->pixmap,
                   line_gc,
                   p1->x, p1->y,
                   p2->x, p2->y);
    if (draw) {
        x = (MIN (p1->x, p2->x) - 2);
        y = (MIN (p1->y, p2->y) - 2);
        width  = abs (p1->x - p2->x) + (2 * 2);
        height = abs (p1->y - p2->y) + (2 * 2);

        gtk_widget_queue_draw_area (widget, x, y, width, height);
    }
}

static void
tomoe_canvas_draw_background (TomoeCanvas *canvas, gboolean draw)
{
    TomoeCanvasPriv *priv;
    GtkWidget *widget;

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    widget = GTK_WIDGET (canvas);

    gdk_draw_rectangle (priv->pixmap,
                        widget->style->white_gc,
                        TRUE,
                        0, 0,
                        widget->allocation.width,
                        widget->allocation.height);

    tomoe_canvas_draw_axis (canvas);

    if (draw) {
        gdk_draw_drawable(widget->window,
                          widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                          priv->pixmap,
                          0, 0,
                          0, 0,
                          widget->allocation.width, widget->allocation.height);
    }
}

static void
tomoe_canvas_draw_axis (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    GtkWidget *widget = GTK_WIDGET (canvas);

    if (!priv->axis_gc) {
        /* FIXME: customize color */
        GdkColor color = { 0, 0x8000, 0x8000, 0x8000 };
        priv->axis_gc = gdk_gc_new (widget->window);
        tomoe_canvas_set_axis_color (canvas, &color);
        gdk_gc_set_line_attributes (priv->axis_gc, 1,
                                    GDK_LINE_ON_OFF_DASH,
                                    GDK_CAP_BUTT,
                                    GDK_JOIN_ROUND);
    }

    gdk_draw_line (priv->pixmap, priv->axis_gc,
                   (priv->width / 2), 0,
                   (priv->width / 2), priv->height);
    gdk_draw_line (priv->pixmap, priv->axis_gc,
                   0,          (priv->height / 2),
                   priv->width, (priv->height / 2));
}

static void
tomoe_canvas_refresh (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv;
    GtkWidget *widget;
    guint i;
    const GList *strokes, *list;

    g_return_if_fail (TOMOE_IS_CANVAS (canvas));

    _init_gc (canvas);

    priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    widget = GTK_WIDGET (canvas);

    gdk_draw_rectangle (priv->pixmap,
                        widget->style->white_gc,
                        TRUE,
                        0, 0,
                        widget->allocation.width,
                        widget->allocation.height);

    tomoe_canvas_draw_axis (canvas);
    /*if(tomoe_writing_get_n_strokes(priv->writing)==4)
    {
        writing=_tomoe_writing_downsample_threshold(priv->writing);
        if (priv->writing)
            g_object_unref (priv->writing);
        priv->writing=writing;
    }*/
    if (priv->writing) {
        strokes = tomoe_writing_get_strokes (priv->writing);
        for (list = strokes, i = 1; list; list = g_list_next (list), i++) {
            GList *points = (GList *) list->data;
            draw_stroke (points, canvas, i);
        }
    }

    gdk_draw_drawable(widget->window,
                      widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                      priv->pixmap,
                      0, 0,
                      0, 0,
                      widget->allocation.width, widget->allocation.height);
}

static void
_init_gc (TomoeCanvas *canvas)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    GtkWidget *widget = GTK_WIDGET (canvas);

    if (!priv->adjusted_line_gc) {
        GdkColor color = { 0, 0x8000, 0x0000, 0x0000 };
        priv->adjusted_line_gc = gdk_gc_new (widget->window);
        tomoe_canvas_set_adjusted_line_color (canvas, &color);
        gdk_gc_set_line_attributes (priv->adjusted_line_gc, 1,
                                    GDK_LINE_SOLID,
                                    GDK_CAP_BUTT,
                                    GDK_JOIN_BEVEL);
    }

    if (!priv->handwriting_line_gc) {
        GdkColor color = { 0, 0x0000, 0x0000, 0x0000 };
        priv->handwriting_line_gc = gdk_gc_new (widget->window);
        tomoe_canvas_set_handwriting_line_color (canvas, &color);
        gdk_gc_set_line_attributes (priv->handwriting_line_gc, 4,
                                    GDK_LINE_SOLID,
                                    GDK_CAP_ROUND,
                                    GDK_JOIN_ROUND);
    }

    if (!priv->annotation_gc) {
        GdkColor color = { 0, 0x8000, 0x0000, 0x0000 };
        priv->annotation_gc = gdk_gc_new (widget->window);
        tomoe_canvas_set_annotation_color (canvas, &color);
    }
}

static void
draw_stroke (GList *points, TomoeCanvas *canvas, guint index)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    GList *node;

    _init_gc (canvas);

    for (node = points; node; node = g_list_next (node)) {
        GList *next = g_list_next (node);
        TomoePoint *p1, *p2;

        if (!next) break;

        p1 = (TomoePoint *) node->data;
        p2 = (TomoePoint *) next->data;

        tomoe_canvas_draw_line (canvas, p1, p2,
                                priv->handwriting_line_gc,
                                FALSE);
    }
    draw_annotation (points, canvas, index);
}

static void
draw_annotation (GList *points, TomoeCanvas *canvas, guint index)
{
    TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    GtkWidget *widget = GTK_WIDGET (canvas);
    gchar *buffer;
    PangoLayout *layout;
    gint width, height;
    gint x, y;
    gdouble r;
    gdouble dx,dy;
    gdouble dl;
    gint sign;

    x = ((TomoePoint*)(points->data))->x;
    y = ((TomoePoint*)(points->data))->y;

    if (g_list_length (points) == 1) {
        dx = x;
        dy = y;
    } else {
        dx = ((TomoePoint*)((g_list_last (points))->data))->x - x;
        dy = ((TomoePoint*)((g_list_last (points))->data))->y - y;
    }

    dl = sqrt (dx*dx + dy*dy);
    sign = (dy <= dx) ? 1 : -1;

    buffer = g_strdup_printf ("%d", index);
    layout = gtk_widget_create_pango_layout (widget, buffer);
    pango_layout_get_pixel_size (layout, &width, &height);

    r = sqrt (width*width + height*height);

    x += (0.5 + (0.5 * r * dx / dl) + (sign * 0.5 * r * dy / dl) - (width / 2));
    y += (0.5 + (0.5 * r * dy / dl) - (sign * 0.5 * r * dx / dl) - (height / 2));

    gdk_draw_layout (priv->pixmap,
                     priv->annotation_gc,
                     x, y, layout);

    g_free (buffer);
    g_object_unref (layout);
}

void
tomoe_reduce(GtkWidget *widget)
{
    //TomoeWriting *writing;
    TomoeCanvas *canvas = TOMOE_CANVAS (widget);
    //TomoeCanvasPriv *priv = TOMOE_CANVAS_GET_PRIVATE (canvas);
    /*writing=_tomoe_writing_get_delta_values(priv->writing);
    if (priv->writing)
            g_object_unref (priv->writing);
    priv->writing=writing;*/
    tomoe_canvas_normalize(canvas);
    tomoe_canvas_refresh (canvas);

}

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
