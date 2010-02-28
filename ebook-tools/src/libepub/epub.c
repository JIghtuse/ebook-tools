#include "epub.h"
#include "epublib.h"
#include <stdarg.h>

struct epub *epub_open(const char *filename, int debug) {
  char *opfName = NULL;
  char *opfStr = NULL;

  struct epub *epub = malloc(sizeof(struct epub));
  if (! epub) {
    _epub_print_debug(NULL, DEBUG_ERROR, "out of memory for allocating a 'epub' object.");
    return NULL;
  }
  epub->ocf = NULL;
  epub->opf = NULL;

  epub->error = malloc(sizeof(struct epuberr));
  if (! epub->error) {
    _epub_print_debug(epub, DEBUG_ERROR, "out of memory for allocating a 'epuberr' object.");
    epub_close(epub);
    return NULL;
  }
  epub->debug = debug;
  _epub_print_debug(epub, DEBUG_INFO, "opening %s", filename);
  
  LIBXML_TEST_VERSION;
  
  if (! (epub->ocf = _ocf_parse(epub, filename))) {
    epub_close(epub);
    return NULL;
  }

  opfName = _ocf_root_fullpath_by_type(epub->ocf, 
                                             "application/oebps-package+xml");
  if (!opfName) {
    epub_close(epub);
    return NULL;
  }

  epub->ocf->datapath = malloc(sizeof(char) *(strlen(opfName) +1));
  char *index = strrchr(opfName, PATH_SEPARATOR);
  if (index) {
    strncpy(epub->ocf->datapath, opfName, index + 1 - opfName); 
    epub->ocf->datapath[index - opfName + 1] = 0;
  }    
  else {
    epub->ocf->datapath[0] = 0;
  }

  _epub_print_debug(epub, DEBUG_INFO, "data path is %s", epub->ocf->datapath );

  _ocf_get_file(epub->ocf, opfName, &opfStr);
  free(opfName);
    

  if (!opfStr) {
    epub_close(epub);
    return NULL;
  }

  epub->opf = _opf_parse(epub, opfStr);
  
  free(opfStr);

  return epub;
}

xmlChar *_getXmlStr(void *str) {
  return xmlStrdup((xmlChar *)str); 
}

xmlChar *_getIdStr(void *id) {
  struct id *data = (struct id *)id;
  xmlChar buff[10000];

  xmlStrPrintf(buff, 10000, (xmlChar *)"%s (%s:%s)", 
               ((data->scheme)?data->scheme:(xmlChar *)"Unspecified"), 
               (data->id?data->id:(xmlChar *)"Unspecified"),
                data->string);
  
  return xmlStrdup(buff);
}

xmlChar *_getDateStr(void *date) {
  struct date *data = (struct date *)date;
  xmlChar buff[10000];

  xmlStrPrintf(buff, 10000, (xmlChar *)"%s: %s", 
               ((data->event)?data->event:(xmlChar *)"Unspecified"), 
               data->date);

  return xmlStrdup(buff);
}

xmlChar *_getMetaStr(void *meta) {
  struct meta *data = (struct meta *)meta;
  xmlChar buff[10000];

  xmlStrPrintf(buff, 10000, (xmlChar *)"%s: %s", 
               ((data->name)?data->name:(xmlChar *)"Unspecified"),
               ((data->content)?data->content:(xmlChar *)"Unspecified"));
  
  return xmlStrdup(buff);
}

xmlChar *_getRoleStr(void *creator) {
  struct creator *data = (struct creator *)creator;
  xmlChar buff[10000];
  xmlStrPrintf(buff, 10000, (xmlChar *)"%s: %s(%s)", 
               ((data->role)?data->role:(xmlChar *)"Author"), 
               data->name, ((data->fileAs)?data->fileAs:data->name));
  
  return xmlStrdup(buff);
}

xmlChar **epub_get_metadata(struct epub *epub, enum epub_metadata type, 
                            int *size) {
  xmlChar **data = NULL;
  listPtr list;
  xmlChar *(*getStr)(void *);
  int i;

  switch(type) {
  case EPUB_ID:
    list = epub->opf->metadata->id;
    getStr = _getIdStr;
    break;
  case EPUB_TITLE:
    list = epub->opf->metadata->title;
    getStr = _getXmlStr;
    break;
  case EPUB_SUBJECT:
    list = epub->opf->metadata->subject;
    getStr = _getXmlStr;
    break;
  case EPUB_PUBLISHER:
    list = epub->opf->metadata->publisher;
    getStr = _getXmlStr;
    break;
  case EPUB_DESCRIPTION:
    list = epub->opf->metadata->description;
    getStr = _getXmlStr;
    break;
  case EPUB_DATE:
    list = epub->opf->metadata->date;
    getStr = _getDateStr;
    break;
  case EPUB_TYPE:
    list = epub->opf->metadata->type;
    getStr = _getXmlStr;
    break;
  case EPUB_FORMAT:
    list = epub->opf->metadata->format;
    getStr = _getXmlStr;
    break;
  case EPUB_SOURCE:
    list = epub->opf->metadata->source;
    getStr = _getXmlStr;
    break;
  case EPUB_LANG:
    list = epub->opf->metadata->lang;
    getStr = _getXmlStr;
    break;
  case EPUB_RELATION:
    list = epub->opf->metadata->relation;
    getStr = _getXmlStr;
    break;
  case EPUB_COVERAGE:
    list = epub->opf->metadata->coverage;
    getStr = _getXmlStr;
    break;
  case EPUB_RIGHTS:
    list = epub->opf->metadata->rights;
    getStr = _getXmlStr;
    break;
  case EPUB_CREATOR:
    list = epub->opf->metadata->creator;
    getStr = _getRoleStr;
    break;
  case EPUB_CONTRIB:
    list = epub->opf->metadata->contrib;
    getStr = _getRoleStr;
    break;
  case EPUB_META:
    list = epub->opf->metadata->meta;
    getStr = _getMetaStr;
    break;
  }

  *size = list->Size;
  if (list->Size <= 0)
    return NULL;

  data = malloc(list->Size * sizeof(xmlChar *));
  
  list->Current = list->Head;

  data[0] = getStr(GetNode(list));
  for (i=1;i<list->Size;i++) {
    data[i] = getStr(NextNode(list));
  }

  return data;
}

// returns the next node that the iterator should return
// if init also check if the current node is good
// if linear is 0 return non linear else return linear
listnodePtr _get_spine_it_next(listnodePtr curr, int linear, int init) {
  struct spine *node; 

  if (! curr)
    return NULL;
  
  if ( ! init) {
    curr = curr->Next;
  }

  node = (struct spine *)GetNodeData(curr);
  
  while(curr) {
    if (! node)
      return NULL;
    
    node = (struct spine *)GetNodeData(curr);
    
    if (node->linear == linear)
      return curr;
    
    curr = curr->Next;
  }

  return NULL;
}

char *_get_spine_it_url(struct eiterator *it) {
  struct manifest *tmp;
  void *data;
  
  data = GetNodeData(it->curr);
  tmp = _opf_manifest_get_by_id(it->epub->opf, 
                                ((struct spine *)data)->idref);
  
  return (char *)tmp->href;
}

struct eiterator *epub_get_iterator(struct epub *epub, 
                                    enum eiterator_type type, int opt) {

  struct eiterator *it = malloc(sizeof(struct eiterator));
  
  it->type = type;
  it->epub = epub;
  it->opt = opt;
  it->cache = NULL;

  switch (type) {
  case EITERATOR_SPINE:
    it->curr = epub->opf->spine->Head;
    break;
  case EITERATOR_NONLINEAR:
    it->curr = _get_spine_it_next(epub->opf->spine->Head, 0, 1); 
    break;
  case EITERATOR_LINEAR:
    it->curr = _get_spine_it_next(epub->opf->spine->Head, 1, 1); 
    break;
  }


  return it;
}

void epub_free_iterator(struct eiterator *it) {
  if (it->cache)
    free(it->cache);

  free(it);
}


char *epub_it_get_curr_url(struct eiterator *it) {
  switch (it->type) {
  case EITERATOR_SPINE:
  case EITERATOR_NONLINEAR:
  case EITERATOR_LINEAR:
    return _get_spine_it_url(it);
  }
  
  return NULL;
}

char *epub_it_get_curr(struct eiterator *it) {

  if (!it->curr)
    return NULL;

  if (!it->cache) {
       
    switch (it->type) {
    case EITERATOR_SPINE:
    case EITERATOR_NONLINEAR:
    case EITERATOR_LINEAR:
      _ocf_get_data_file(it->epub->ocf, _get_spine_it_url(it), &(it->cache));
      break;
    }
  }
  
  return it->cache;
}
char *epub_it_get_next(struct eiterator *it) {
  
  if (it->cache) {
    free(it->cache);
    it->cache = NULL;
  }

  if (!it->curr)
    return NULL;

  switch (it->type) {

  case EITERATOR_SPINE:
    it->curr = it->curr->Next;
    break;
    
  case EITERATOR_NONLINEAR:
    it->curr = _get_spine_it_next(it->curr, 0, 0); 
    break;

  case EITERATOR_LINEAR:
    it->curr = _get_spine_it_next(it->curr, 1, 0); 
    break;
  }
  
  return epub_it_get_curr(it);
}

int epub_close(struct epub *epub) {
  if (epub->error) 
    free(epub->error);
 
  if (epub->ocf)
    _ocf_close(epub->ocf);

  if (epub->opf)
    _opf_close(epub->opf);

  if (epub)
    free(epub);

  
  return 1;
}

void epub_set_debug(struct epub *epub, int debug) {
  epub->debug = debug;
}

void _epub_print_debug(struct epub *epub, int debug, char *format, ...) {
  va_list ap;
  char strerr[1025];

  va_start(ap, format);

  vsnprintf(strerr, 1024, format, ap);
  strerr[1024] = 0;
  
  if (epub && (debug == DEBUG_ERROR)) {
    epub->error->len = strlen(strerr);
    strcpy(epub->error->lastStr, strerr);
  }

  if (! epub || (epub->debug >= debug)) {
    fprintf(stderr, "libepub ");
    switch(debug) {
    case DEBUG_ERROR: 
      
      fprintf(stderr, "(EE)");
    break;
    case DEBUG_WARNING:
      fprintf(stderr, "(WW)");
      break;
    case DEBUG_INFO:
      fprintf(stderr, "(II)");
      break;
    case DEBUG_VERBOSE:
      fprintf(stderr, "(VV)");
      break;
    }
    fprintf(stderr, ": \t%s\n" , strerr);
  }
  va_end(ap);
}

int epub_tit_next(struct titerator *tit) {
  listnodePtr curr = tit->next;
  if (! curr) {
    tit->valid = 0;
    return 0;
  }

  tit->next = curr->Next;
  
  switch (tit->type) {
    struct guide* guide;
    struct tocItem *ti;

  case TITERATOR_GUIDE:
    guide = GetNodeData(curr);
    tit->cache.label = (char *)guide->title;
    tit->cache.link = (char *)guide->href;
    tit->cache.depth = 1;

    break;

  case TITERATOR_NAVMAP:
  case TITERATOR_PAGES:
    ti = GetNodeData(curr);
    tit->cache.label = 
      (char *)_opf_label_get_by_doc_lang(tit->epub->opf, ti->label);

    if (! tit->cache.label)
      tit->cache.label = (char *)ti->id;

    tit->cache.depth = ti->depth;
    tit->cache.link = (char *)ti->src;
    break;

  }

  tit->valid = 1;
  return 1;
}

struct titerator *epub_get_titerator(struct epub *epub, 
                                     enum titerator_type type, int opt) {
  struct titerator *it;

  switch (type) {
  case TITERATOR_NAVMAP:
    if (! epub->opf->toc || ! epub->opf->toc->navMap)
      return NULL;
    break;
  case TITERATOR_GUIDE:
    if (! epub->opf->guide) 
      return NULL;
    break;
  case TITERATOR_PAGES:
    if (! epub->opf->toc || epub->opf->toc->pageList)
      return NULL;
    break;
  }

  it = malloc(sizeof(struct titerator));
  it->type = type;
  it->epub = epub;
  it->opt = opt;
  it->next = NULL;
  it->valid = 0;

  it->cache.label = NULL;
  it->cache.link = NULL;
  it->cache.depth = -1;


  switch (type) {
  case TITERATOR_NAVMAP:
    it->next = epub->opf->toc->navMap->items->Head;
    if (epub->opf->toc->navMap->label) {
      it->cache.label = 
        (char *)_opf_label_get_by_doc_lang(epub->opf, 
                                           epub->opf->toc->navMap->label);
      it->cache.depth = 0;
    }
    it->valid = 1;
    break;

  case TITERATOR_GUIDE:
    it->next = epub->opf->guide->Head;
    break;
    
  case TITERATOR_PAGES:
    it->next = epub->opf->toc->pageList->items->Head;
    if (epub->opf->toc->pageList->label) {
      it->cache.label = 
        (char *)_opf_label_get_by_doc_lang(epub->opf, 
                                           epub->opf->toc->pageList->label);
      it->cache.depth = 1;
    }
    it->valid = 1;
    break;
  }
  
  if ( ! it->cache.label)
    epub_tit_next(it);
  
  return it;
}

int epub_tit_curr_valid(struct titerator *tit) {
  return tit->valid;
}

char *epub_tit_get_curr_label(struct titerator *tit) {
  // FIXME how can there be unlabeled curr?
  return tit->cache.label?strdup(tit->cache.label):NULL;
}

int epub_tit_get_curr_depth(struct titerator *tit) {
  return tit->cache.depth;
}

char *epub_tit_get_curr_link(struct titerator *tit) {
  return strdup(tit->cache.link);

}

void epub_free_titerator(struct titerator *tit) {

  free(tit);
}
  
int epub_get_ocf_file(struct epub *epub, const char *filename, char **data) {
  return _ocf_get_file(epub->ocf, filename, data);
}

int epub_get_data(struct epub *epub, const char *name, char **data) {
  return _ocf_get_data_file(epub->ocf, name, data);
}

void epub_dump(struct epub *epub) {
  _ocf_dump(epub->ocf);
  _opf_dump(epub->opf);
}

void epub_cleanup() {
  xmlCleanupParser();
}

char *epub_last_errStr(struct epub *epub) {
  char *errStr = epub->error->lastStr;
  char *res = malloc(epub->error->len +1);
  strcpy(errStr, res);

  return res;
}

