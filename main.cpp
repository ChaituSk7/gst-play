#include "header.h"

int main (int argc, char *argv[]) {

  for (int index = 1; index < argc; index++) {
    char *path = realpath(argv[index], NULL);
    if (path != NULL) {
      gchar *uri = gst_filename_to_uri(path, NULL);
      main_pipeline(uri);
    }
  }
  return 0;
}