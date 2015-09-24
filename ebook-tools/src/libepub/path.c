#include "path.h"
#include <string.h>

// We need canonicalize_file_name (3) equivalent for files in zip archive:
// process "..", "//", but not symlinks/hardlinks
// TODO: Add tests. Add more tests.
// TODO: get rid of buffer?
// TODO: simplify it?
#define MAX_PATH_SIZE 4096

char* canonicalize_filename(const char *str)
{
  int prev_last_slash = -1;
  int last_slash = -1;
  int i = 0;
  int j = 0;
  char c;
  char cprev = '\0';
  char result[MAX_PATH_SIZE] = {0};

  if (!str)
    return NULL;

  for (; i < MAX_PATH_SIZE && str[i]; ++i) {
    c = str[i];

    switch (c) {
    case '/':
      if (cprev == '/' || j == 0) {
        // skip entirely
        ;
      } else {
        result[j++] = c;
        prev_last_slash = last_slash;
        last_slash = i;
      }
      break;
    case '.':
      if (cprev == '.') {
        int start_position = 0;
        if (prev_last_slash > 0) {
          start_position = prev_last_slash;
          // handle following duplicate slash on next iteration, if any
          cprev = '/';
        }
        while (j > start_position)
          result[j--] = '\0';
        result[j] = '\0';
      } else {
        // assume it is a valid to have dot in names
        result[j++] = c;
      }
      break;
    default:
      result[j++] = c;
      break;
    }
    cprev = c;
  }
  return strndup(result, MAX_PATH_SIZE - 1);
}
