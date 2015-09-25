#ifndef PATH_H
#define PATH_H 1

#ifdef __cplusplus
extern "C" {
#endif

// canonicalizes filename: remove repeating and leading "/", handle ".."
// allocates string which should be free()d
char* canonicalize_filename(const char *str);

#ifdef __cplusplus
}
#endif

#endif // PATH_H
