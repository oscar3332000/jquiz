/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  2010 Oscar García
 *
 * This program is free software; you can redistribute it and/or
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

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <glib.h>
#include "quiz_frame.h"


gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}


void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

   //We can now link this pad with the vorbis-decoder sink pad 
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (decoder, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}

GMainLoop *loop;
GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
GstBus *bus;


int main(int argc, char *argv[])
{
   QuizFrame *window;

    gtk_init (&argc, &argv);
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);


   /*if( !g_thread_supported() )
   {
	    g_thread_init(NULL);
	    gdk_threads_init();                   // Called to initialize internal mutex "gdk_threads_mutex".
	    g_print("g_thread supported\n");
   }*/

    //  Create gstreamer elements 
    pipeline = gst_pipeline_new ("audio-player");
    source   = gst_element_make_from_uri (GST_URI_SRC,"http://translate.google.com/" ,NULL);
    decoder  = gst_element_factory_make ("mad",     "vorbis-decoder");
    conv     = gst_element_factory_make ("audioconvert",  "converter");
    sink     = gst_element_factory_make ("autoaudiosink", "audio-output");

    g_object_set (G_OBJECT (source), "location", "http://translate.google.com/translate_tts?tl=ja&ie=utf-8&q=オスカル", NULL);

     //we add a message handler 
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    gst_bin_add_many (GST_BIN (pipeline),
                      source, decoder, conv, sink, NULL);
    gst_element_link_many (source, decoder, sink, conv, NULL);

   

   g_object_set (gtk_settings_get_default (), "gtk-button-images", TRUE, NULL);
   window = quiz_frame_new();
   gtk_window_resize (GTK_WINDOW (window), 800, 700);
   gtk_signal_connect (GTK_OBJECT (window), "destroy",GTK_SIGNAL_FUNC (gtk_exit), NULL);
   gtk_container_border_width (GTK_CONTAINER (window), 10);
   gtk_widget_show(GTK_WIDGET(window));
   gtk_main();
   return 0;
}
/*
 * vi:ts=4:nowrap:ai:expandtab
 */
