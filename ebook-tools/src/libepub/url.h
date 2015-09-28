#ifndef URL_H
#define URL_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

// simplified version of base64. Modifies @str in-place.
// For now just replaces "%20" with " ".
void url_decode(char *str, size_t len);

#ifdef __cplusplus
}
#endif

#endif // URL_H