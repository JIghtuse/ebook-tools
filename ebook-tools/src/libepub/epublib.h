#ifndef EPUBLIB_H
#define EPUBLIB_H 1
// generally needed includes
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// For opening the zip file
#include <zip.h>
#include <zlib.h>

// For parsing xml
#include <libxml/xmlreader.h>

// For list stuff
#include "linklist.h"
#include "epub_shared.h"

// General definitions
#ifdef _WIN32
# define PATH_SEPARATOR '\\'
#else
# define PATH_SEPARATOR '/'
#endif

// MSVC-specific definitions
#ifdef _MSC_VER
# define strdup _strdup
#endif

///////////////////////////////////////////////////////////
// OCF definions
///////////////////////////////////////////////////////////
#define METAINFO_DIR "META-INF"
#define MIMETYPE_FILENAME "mimetype"
#define CONTAINER_FILENAME "container.xml"
#define MANIFEST_FILENAME  "manifest.xml"
#define METADATA_FILENAME  "metadata.xml"
#define SIGNATURES_FILENAME "signatures.xml"
#define ENCRYPTION_FILENAME "encryption.xml"
#define RIGHTS_FILENAME "rights.xml"

// An OCF root 
struct root {
  xmlChar *mediatype; // media type (mime)
  xmlChar *fullpath; // full path to the root
};


struct ocf {
  char *datapath; // The path that the data files relative to 
  char *filename; // The ebook filename
  struct zip *arch; // The epub zip
  char *mimetype; // For debugging 
  listPtr roots; // list of OCF roots
  struct epub *epub; // back pointer
};

struct meta {
  xmlChar *name;
  xmlChar *content;
};

struct id {
  xmlChar *id;
  xmlChar *scheme;
  xmlChar *string;
};

struct date {
  xmlChar *date;
  xmlChar *event;
};

struct creator {
  xmlChar *name;
  xmlChar *fileAs;
  xmlChar *role;
};

struct metadata {
  listPtr id;
  listPtr title;
  listPtr creator;
  listPtr contrib;
  listPtr subject;
  listPtr publisher;
  listPtr description;
  listPtr date;
  listPtr type;
  listPtr format;
  listPtr source;
  listPtr lang;
  listPtr relation;
  listPtr coverage;
  listPtr rights;
  listPtr meta;
};

struct manifest {
  xmlChar *nspace; 
  xmlChar *modules; 
  xmlChar *id;
  xmlChar *href;
  xmlChar *type;
  xmlChar *fallback;
  xmlChar *fbStyle;

};
    
struct guide {
  xmlChar *type;
  xmlChar *title;
  xmlChar *href;
};

struct site {
  xmlChar *title;
  xmlChar *href;
};

struct tour {
  xmlChar *id;
  xmlChar *title;
  listPtr sites;
};

// Struct for navLabel and navInfo
struct tocLabel {
  xmlChar *lang;
  xmlChar *dir;
  xmlChar *text;
};

// struct for navPoint, pageTarget, navTarget 
struct tocItem {
  xmlChar *id;
  xmlChar *src;
  xmlChar *class;
  xmlChar *type; //pages
  listPtr label;
  int depth;
  int playOrder;
  int value;
};

// struct for navMap, pageList, navList
struct tocCategory {
  xmlChar *id;
  xmlChar *class;
  listPtr info; //tocLabel
  listPtr label; //tocLabel
  listPtr items; //tocItem
};

// General toc struct
struct toc {
  struct tocCategory *navMap; 
  struct tocCategory *pageList;
  struct tocCategory *navList;
  listPtr playOrder;
};

struct spine {
  xmlChar *idref;
  int linear; //bool
};

struct opf {
  char *name;
  xmlChar *tocName;
  struct epub *epub;
  struct metadata *metadata;
  struct toc *toc; // must in opf 2.0
  listPtr manifest;
  listPtr spine;
  int linearCount;
    
  // might be NULL
  listPtr guide;
  listPtr tours;
};

struct epuberr {
  char lastStr[1025];
  int len;
}; 

// general structs
struct epub {
  struct ocf *ocf;
  struct opf *opf;
  struct epuberr *error;
  int debug;

};

enum {
  DEBUG_NONE,
  DEBUG_ERROR,
  DEBUG_WARNING,
  DEBUG_INFO,
  DEBUG_VERBOSE
};

struct eiterator {
  enum eiterator_type type;
  struct epub *epub;
  int opt;
  listnodePtr curr;
  char *cache;
};

struct tit_info {
  char *label;
  int depth;
  char *link;
};

struct titerator {
  enum titerator_type type;
  struct epub *epub;
  int opt;
  listnodePtr next;
  struct tit_info cache;
  int valid;
};

// Ocf functions
struct ocf *_ocf_parse(struct epub *epub, const char *filename);
void _ocf_dump(struct ocf *ocf);
void _ocf_close(struct ocf *ocf);
struct zip *_ocf_open(struct ocf *ocf, const char *fileName);
int _ocf_get_file(struct ocf *ocf, const char *filename, char **fileStr);
int _ocf_get_data_file(struct ocf *ocf, const char *filename, char **fileStr);
int _ocf_check_file(struct ocf *ocf, const char *filename);
char *_ocf_root_by_type(struct ocf *ocf, char *type);
char *_ocf_root_fullpath_by_type(struct ocf *ocf, char *type);

// Parsing ocf
int _ocf_parse_container(struct ocf *ocf);
int _ocf_parse_mimetype(struct ocf *ocf);

// parsing opf
struct opf *_opf_parse(struct epub *epub, char *opfStr);
void _opf_dump(struct opf *opf);
void _opf_close(struct opf *opf);

void _opf_parse_metadata(struct opf *opf, xmlTextReaderPtr reader);
void _opf_parse_spine(struct opf *opf, xmlTextReaderPtr reader);
void _opf_parse_manifest(struct opf *opf, xmlTextReaderPtr reader);
void _opf_parse_guide(struct opf *opf, xmlTextReaderPtr reader);
void _opf_parse_tours(struct opf *opf, xmlTextReaderPtr reader);

// parse toc
void _opf_parse_toc(struct opf *opf, char *tocStr, int size);
void _opf_parse_navlist(struct opf *opf, xmlTextReaderPtr reader);
void _opf_parse_navmap(struct opf *opf, xmlTextReaderPtr reader);
void _opf_parse_pagelist(struct opf *opf, xmlTextReaderPtr reader);
struct tocLabel *_opf_parse_navlabel(struct opf *opf, xmlTextReaderPtr reader);
void _opf_free_toc_category(struct tocCategory *tc);
void _opf_free_toc(struct toc *toc);
struct toc *_opf_init_toc();
struct tocCategory *_opf_init_toc_category();

xmlChar *_opf_label_get_by_lang(struct opf *opf, listPtr label, char *lang);
xmlChar *_opf_label_get_by_doc_lang(struct opf *opf, listPtr label);

struct manifest *_opf_manifest_get_by_id(struct opf *opf, xmlChar* id);

// epub functions
struct epub *epub_open(const char *filename, int debug);
void _epub_print_debug(struct epub *epub, int debug, char *format, ...);
char *epub_last_errStr(struct epub *epub);
void _epub_print_debug(struct epub *epub, int debug, char *format, ...);

// List operations
void _list_free_root(struct root *data);

void _list_free_creator(struct creator *data);
void _list_free_date(struct date *date);
void _list_free_id(struct id *id);
void _list_free_meta(struct meta *meta);

void _list_free_spine(struct spine *spine);
void _list_free_manifest(struct manifest *manifest);
void _list_free_guide(struct guide *guide);
void _list_free_tours(struct tour *tour);

void _list_free_toc_label(struct tocLabel *tl);
void _list_free_toc_item(struct tocItem *ti);

int _list_cmp_root_by_mediatype(struct root *root1, struct root *root2);
int _list_cmp_manifest_by_id(struct manifest *m1, struct manifest *m2);
int _list_cmp_toc_by_playorder(struct tocItem *t1, struct tocItem *t2);
int _list_cmp_label_by_lang(struct tocLabel *t1, struct tocLabel *t2);

void _list_dump_root(struct root *root);

void _list_dump_string(char *string);
void _list_dump_creator(struct creator *data);
void _list_dump_date(struct date *date);
void _list_dump_id(struct id *id);
void _list_dump_meta(struct meta *meta);

void _list_dump_spine(struct spine *spine);
void _list_dump_guide(struct guide *guide);
void _list_dump_tour(struct tour *tour);

#endif /* epublib_h */ 
