#ifndef HEADER_H
#define HEADER_H
#include <gst/gst.h>

typedef struct _CustomData {
  GstElement *pipeline;
  GstElement *video_sink;
  GMainLoop *loop;
  gboolean playing;
  gdouble rate;
  gdouble volume;
}CustomData;


extern void main_pipeline(char *);

#endif