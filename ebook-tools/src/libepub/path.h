#ifndef PATH_H
#define PATH_H 1

// canonicalizes filename: remove repeating and leading "/", handle ".."
// allocates string which should be free()d
char* canonicalize_filename(const char *str);

#endif // PATH_H
