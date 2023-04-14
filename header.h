#include <gst/gst.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>

#define MAX_SIZE 1024

typedef struct _CommandLineArgs {
  int argc;
  char *args[MAX_SIZE];
  GMainLoop *loop;
}CommandLineArgs;

int tutorial_main (int, char *[]);