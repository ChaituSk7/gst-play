#include <stdio.h>
#include "header.h"

/* Send seek event to change rate */
static void send_seek_event (CustomData * data) {
  gint64 position;
  GstEvent *seek_event;

  /* Obtain the current position, needed for the seek event */
  if (!gst_element_query_position (data->pipeline, GST_FORMAT_TIME, &position)) {
    g_printerr ("Unable to retrieve current position.\n");
    return;
  }
  /* Create the seek event */
  if (data->rate > 0) {
    seek_event =
        gst_event_new_seek (data->rate, GST_FORMAT_TIME,
        (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), GST_SEEK_TYPE_SET,
        position, GST_SEEK_TYPE_END, 0);
  } 
  else {
    seek_event =
        gst_event_new_seek (data->rate, GST_FORMAT_TIME,
        (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), GST_SEEK_TYPE_SET, 0,
        GST_SEEK_TYPE_SET, position);
  }

  if (data->video_sink == NULL) {
    /* If we have not done so, obtain the sink through which we will send the seek events */
    g_object_get (data->pipeline, "video-sink", &data->video_sink, NULL);
  }

  /* Send the event */
  gst_element_send_event (data->video_sink, seek_event);

  g_print ("Playback rate: %g\n", data->rate);
}


static gboolean handle_keyboard (GIOChannel * source, GIOCondition cond, CustomData * data) {
  gchar *str = NULL;

  if (g_io_channel_read_line (source, &str, NULL, NULL,
          NULL) != G_IO_STATUS_NORMAL) {
    return TRUE;
  }

  switch (g_ascii_tolower (str[0])) {
    case 'p':
      data->playing = !data->playing;
      gst_element_set_state (data->pipeline,
          data->playing ? GST_STATE_PLAYING : GST_STATE_PAUSED);
      g_print ("Setting state to %s\n", data->playing ? "PLAYING" : "PAUSE");
      break;
    case '+':
      data->rate = data->rate + 0.10;
      send_seek_event (data);
      break;
    case '-':
      data->rate = data->rate - 0.10;
      send_seek_event(data);
      break;
    case 'd':
      data->rate *= -1.0;
      send_seek_event (data);
      break;
    case 'k':
       g_print ("USAGE: Choose one of the following options, then press enter:\n"
      "     p    : PAUSE/PLAY\n"
      "     q    : quit\n" 
      "     +    : increase playback rate\n"
      "     -    : decrease playback rate\n"
      "     l    : seek forward (10 sec)\n"
      "     j    : seek backward (10 sec)\n"
      "     d    : toggle playback direction\n"
      "     n    : play next\n"
      "     v    : increase volume\n"
      "     u    : decrease volume\n"
      "     0    : seek to begining\n"
      "     k    : show keyboard shortcuts\n");
      break;
    case 'q':
      g_main_loop_quit (data->loop);
      exit(0);
      break;
    case 'n':
      gst_element_set_state(data->pipeline, GST_STATE_PAUSED);
	  	gst_event_new_seek (1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_SET, 0);
	  	gst_element_set_state(data->pipeline, GST_STATE_NULL);
	  	g_main_loop_quit(data->loop);
      break;
    case 'v':
      data->volume += 1.0;
      g_object_set(G_OBJECT(data->pipeline), "volume", data->volume, NULL);
      g_print("volume %f\n", data->volume);
      break;
    case 'u':
      data->volume -= 1.0;
      g_object_set(G_OBJECT(data->pipeline), "volume", data->volume, NULL);
      g_print("volume %f\n", data->volume);
      break;
    case '0':
      gst_element_seek_simple(data->pipeline, GST_FORMAT_TIME,
                      (GstSeekFlags)(GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH), 0);
      break;
    case 'l': {
      gint64 position;
      if (!gst_element_query_position (data->pipeline, GST_FORMAT_TIME, &position)) {
        g_printerr ("Unable to retrieve current position.\n");
        return FALSE;
      }
      gst_element_seek_simple(data->pipeline, GST_FORMAT_TIME,
                      (GstSeekFlags)(GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH), (position + (10*GST_SECOND)));
    }
    break;
    case 'j': {
      gint64 position;
      if (!gst_element_query_position (data->pipeline, GST_FORMAT_TIME, &position)) {
        g_printerr ("Unable to retrieve current position.\n");
        return FALSE;
      }
      if (position )
      gst_element_seek_simple(data->pipeline, GST_FORMAT_TIME,
                      (GstSeekFlags)(GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH), 
                      (position - (10*GST_SECOND)));
      }
    default:
      break;
  }

  g_free (str);

  return TRUE;
}

static void callback_message(GstBus *bus, GstMessage *msg, CustomData *data) {
  switch(GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_UNKNOWN:
			g_print("\nUnknown Message Received.\n");
			break;		
		case GST_MESSAGE_EOS:
			g_print("\nEnd of Stream Reached.\n");
			gst_element_set_state(data->pipeline, GST_STATE_NULL);
			g_main_loop_quit(data->loop);
			break;
					
		case GST_MESSAGE_ERROR:	{
      GError *error = NULL;
      gchar *debug = NULL;
      gst_message_parse_error(msg, &error, &debug);
      g_print("\n Error received from %s : %s", GST_OBJECT_NAME(msg->src), error->message);
      g_print("\n Debug Info : %s\n",(debug)? debug : "None");
      g_error_free(error);
      g_free(debug);
      gst_element_set_state(data->pipeline, GST_STATE_NULL);
      g_main_loop_quit(data->loop);
    }
    break;
		default:
			break;
	}
}

void main_pipeline(char *file) {
  GIOChannel *io_stdin;
  CustomData data;
  GstStateChangeReturn ret;
  GstBus *bus;

  memset(&data, 0, sizeof(data));

  /* Initialize gstreamer */
  gst_init(NULL, NULL);

  data.pipeline = gst_element_factory_make("playbin", NULL);

  if (!data.pipeline) {
    g_printerr("Playbin not created.\n");
  }

  g_object_set(G_OBJECT(data.pipeline), "uri", file, NULL);

  ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);

  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr("Failed to set the pipeline to playing state.\n");
    exit(EXIT_FAILURE);
  }
  g_print("Press 'k' to see a list of keyboard shortcuts.\n");
  g_print("Now playing %s\n", file);

  data.playing = TRUE;
  data.rate = 1.0;
  data.volume = 1.0;

  #ifdef G_OS_WIN32
  io_stdin = g_io_channel_win32_new_fd (fileno (stdin));
  #else
    io_stdin = g_io_channel_unix_new (fileno (stdin));
  #endif
    guint id = g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc) (handle_keyboard), &data);

  data.loop = g_main_loop_new(NULL, FALSE);
 
  bus = gst_element_get_bus(data.pipeline);
  gst_bus_add_signal_watch(bus);

  g_signal_connect(bus, "message", G_CALLBACK(callback_message), &data);

  g_main_loop_run(data.loop);

  g_source_remove(id);
  g_main_loop_unref(data.loop);
  gst_element_set_state(data.pipeline, GST_STATE_NULL);
  gst_object_unref(data.pipeline);
  return;
}