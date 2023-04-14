#include "header.h"

static gboolean handle_keyboard (GIOChannel *source, GIOCondition cond, CommandLineArgs *data) {
  gchar *str = NULL;
  if (g_io_channel_read_line(source, &str, NULL, NULL, NULL) != G_IO_STATUS_NORMAL) {
    return true;
  }
  switch(g_ascii_tolower(str[0])) {
    case 'k' : 
      g_print ("Interactive mode - keyboard controls:\n\n"
      "     space     : pause/unpause\n"
      "     q or ESC  : quit\n"
      "     > or n    : play next\n"
      "     ->        : seek forward (10 sec)\n"
      "     <-        : seek backward (10 sec)\n"
      "     k         : show keyboard shorcuts\n");
      break;
    case 'n':
      for (int index = 1 ; index < data->argc; index++) {
        gchar *uri = gst_filename_to_uri(data->args[index], NULL);
        g_print("%s\n", uri);
        g_free(uri);
      }
      g_main_loop_quit(data->loop);
      break;
    default :
      g_printerr("Unexcepted key received\n"); 
      break;
  }

  g_free(str);
  return true;
}

int tutorial_main (int argc, char *argv[]) {
  
  // GstStateChangeReturn ret;
  GIOChannel *io_stdin;
  CommandLineArgs cmd_args;
  
  memset(&cmd_args, 0, sizeof(cmd_args));
  for(int index = 0; index < argc; index++){
    cmd_args.args[index] = (char*)malloc(MAX_SIZE * sizeof(char));
  }

  cmd_args.argc = argc;

  for (int index = 1 ; index < argc; index++) {
    char *path = realpath(argv[index], NULL);
    if (path != NULL) {
      strcpy(cmd_args.args[index], argv[index]);
    }
    else {
      g_printerr("file not found : %s\n", argv[index]);
    }
  }

  
  g_print("Press 'k' to see a list of keyboard shortcuts\n");

  #ifdef G_OS_WIN32
    io_stdin = g_io_channel_win32_new_fd (fileno(stdin));
  #else
    io_stdin = g_io_channel_unix_new(fileno(stdin));
  #endif
    g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc)(handle_keyboard), &cmd_args);

  cmd_args.loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (cmd_args.loop);

  g_main_loop_unref (cmd_args.loop);
  g_io_channel_unref (io_stdin);


  for(int i = 1; i < argc; i++){
    free(cmd_args.args[i]);
  }

  return 0;
}



