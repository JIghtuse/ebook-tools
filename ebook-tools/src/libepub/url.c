#include "url.h"
#include <stdio.h>

// simplified version of base64. Modifies @str in-place.
// For now just replaces "%20" with " ".
// TODO: extend it or use some library
void url_decode(char *str, size_t len)
{
  size_t code_length = 3;
  size_t i;
  size_t current_offset = 0;

  if (str == NULL)
    return NULL;

  for (i = 0; i + current_offset < len && str[i]; ++i) {
    char c = str[i + current_offset];
    str[i] = c;

    if (c == '%') {
      if (!strncmp(&str[i + current_offset], "%20", code_length)) {
       str[i] = ' ';
       current_offset += code_length - 1;
      }
    }
  }
}