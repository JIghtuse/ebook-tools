#include "epublib.h"

struct opf *_opf_parse(struct epub *epub, char *opfStr) {
  _epub_print_debug(epub, DEBUG_INFO, "building opf struct");
  
  struct opf *opf = malloc(sizeof(struct opf));
  opf->epub = epub;
  opf->guide = NULL;
  opf->tours = NULL;
  opf->toc = NULL;
  
 
  xmlTextReaderPtr reader;
  int ret;
  
  reader = xmlReaderForMemory(opfStr, strlen(opfStr), 
                              "OPF", NULL, 0);
   if (reader != NULL) {
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
      const xmlChar *name = xmlTextReaderConstName(reader);
      if (xmlStrcmp(name, (xmlChar *)"metadata") == 0)
        _opf_parse_metadata(opf, reader);
      else 
      if (xmlStrcmp(name, (xmlChar *)"manifest") == 0)
        _opf_parse_manifest(opf, reader);
      else 
      if (xmlStrcmp(name, (xmlChar *)"spine") == 0)
        _opf_parse_spine(opf, reader);
      else 
      if (xmlStrcmp(name, (xmlChar *)"guide") == 0)
        _opf_parse_guide(opf, reader);
      else 
      if (xmlStrcmp(name, (xmlChar *)"tours") == 0)
        _opf_parse_tours(opf, reader);
      
     ret = xmlTextReaderRead(reader);
    }

    xmlFreeTextReader(reader);
    if (ret != 0) {
      _epub_print_debug(opf->epub, DEBUG_ERROR, "failed to parse OPF");
      return NULL;
    }
   } else {
     _epub_print_debug(opf->epub, DEBUG_ERROR, "unable to open OPF");
     return NULL;
   }

   return opf;
}

xmlChar *_get_possible_namespace(xmlTextReaderPtr reader, 
                                 const xmlChar * localName, 
                                 const xmlChar * namespace) 
{
  xmlChar *tmp, *ns;
  ns = xmlTextReaderLookupNamespace(reader, namespace);
  tmp = xmlTextReaderGetAttributeNs(reader, localName, ns);

  if (ns)
    free(ns);
  
  if (! tmp)
    return xmlTextReaderGetAttribute(reader, localName);
  
  return tmp;
}

void _opf_init_metadata(struct opf *opf) {
  struct metadata *meta = malloc(sizeof(struct metadata));

  meta->id = NewListAlloc(LIST, NULL, NULL, NULL);  
  meta->title = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->creator = NewListAlloc(LIST, NULL, NULL, NULL);
  meta->contrib = NewListAlloc(LIST, NULL, NULL, NULL);
  meta->subject = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->publisher = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->description = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->date = NewListAlloc(LIST, NULL, NULL, NULL);
  meta->type = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->format = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->source = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->lang = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->relation = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->coverage = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->rights = NewListAlloc(LIST, NULL, NULL, (NodeCompareFunc)StringCompare);
  meta->meta = NewListAlloc(LIST, NULL, NULL, NULL);

  opf->metadata = meta;
}

void _opf_free_metadata(struct metadata *meta) {
  FreeList(meta->id, (ListFreeFunc)_list_free_id);
  FreeList(meta->title, free);
  FreeList(meta->creator, (ListFreeFunc)_list_free_creator);
  FreeList(meta->contrib, (ListFreeFunc)_list_free_creator);
  FreeList(meta->subject, free);
  FreeList(meta->publisher, free);
  FreeList(meta->description, free);
  FreeList(meta->date, (ListFreeFunc)_list_free_date);
  FreeList(meta->type, free);
  FreeList(meta->format, free);
  FreeList(meta->source, free);
  FreeList(meta->lang, free);
  FreeList(meta->relation, free);
  FreeList(meta->coverage, free);
  FreeList(meta->rights, free);
  FreeList(meta->meta, (ListFreeFunc)_list_free_meta);
  free(meta);
}

void _opf_parse_metadata(struct opf *opf, xmlTextReaderPtr reader) {
  int ret;
  struct metadata *meta;
  
  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing metadata");
  
  // must have title, identifier and language
  _opf_init_metadata(opf);
  meta = opf->metadata;
  
  ret = xmlTextReaderRead(reader);
  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"metadata")) {

    // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }
    
    const xmlChar *local = xmlTextReaderConstLocalName(reader);
    xmlChar *string = (xmlChar *)xmlTextReaderReadString(reader);

    if (xmlStrcasecmp(local, (xmlChar *)"identifier") == 0) {
      struct id *new = malloc(sizeof(struct id));
      new->string = string;
      new->scheme = _get_possible_namespace(reader, (xmlChar *)"scheme",
                                                (xmlChar *)"opf");
      new->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
      
      AddNode(meta->id, NewListNode(meta->id, new));
      _epub_print_debug(opf->epub, DEBUG_INFO, "identifier %s(%s) is: %s", 
                        new->id, new->scheme, new->string);
    } else if (xmlStrcasecmp(local, (xmlChar *)"title") == 0) {
      AddNode(meta->title, NewListNode(meta->title, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "title is %s", string);
        
    } else if (xmlStrcasecmp(local, (xmlChar *)"creator") == 0) {
      struct creator *new = malloc(sizeof(struct creator));
      new->name = string;
      new->fileAs = 
        _get_possible_namespace(reader, (xmlChar *)"file-as",
                                    (xmlChar *)"opf");
      new->role = 
        _get_possible_namespace(reader, (xmlChar *)"role",
                                    (xmlChar *)"opf");
      AddNode(meta->creator, NewListNode(meta->creator, new));       
      _epub_print_debug(opf->epub, DEBUG_INFO, "creator - %s: %s (%s)", 
                        new->role, new->name, new->fileAs);
        
    } else if (xmlStrcasecmp(local, (xmlChar *)"contributor") == 0) {
      struct creator *new = malloc(sizeof(struct creator));
      new->name = string;
      new->fileAs = 
        _get_possible_namespace(reader, (xmlChar *)"file-as",
                                    (xmlChar *)"opf");
      new->role = 
        _get_possible_namespace(reader, (xmlChar *)"role",
                                    (xmlChar *)"opf");
      AddNode(meta->contrib, NewListNode(meta->contrib, new));     
      _epub_print_debug(opf->epub, DEBUG_INFO, "contributor - %s: %s (%s)", 
                        new->role, new->name, new->fileAs);
      
    } else if (xmlStrcasecmp(local, (xmlChar *)"meta") == 0) {
      struct meta *new = malloc(sizeof(struct meta));
      new->name = xmlTextReaderGetAttribute(reader, (xmlChar *)"name");
      new->content = xmlTextReaderGetAttribute(reader, (xmlChar *)"content");
      
      AddNode(meta->meta, NewListNode(meta->meta, new));
      if (string)
        free(string);
      _epub_print_debug(opf->epub, DEBUG_INFO, "meta is %s: %s", 
                        new->name, new->content); 
    } else if (xmlStrcasecmp(local, (xmlChar *)"date") == 0) {
      struct date *new = malloc(sizeof(struct date));
      new->date = string;
      new->event = _get_possible_namespace(reader, (xmlChar *)"event",
                                               (xmlChar *)"opf");
      AddNode(meta->date, NewListNode(meta->date, new));
      _epub_print_debug(opf->epub, DEBUG_INFO, "date is %s: %s", 
                        new->event, new->date); 
        
    } else if (xmlStrcasecmp(local, (xmlChar *)"subject") == 0) {
      AddNode(meta->subject, NewListNode(meta->subject, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "subject is %s", string);
        
    } else if (xmlStrcasecmp(local, (xmlChar *)"publisher") == 0) {
      AddNode(meta->publisher, NewListNode(meta->publisher, string)); 
      _epub_print_debug(opf->epub, DEBUG_INFO, "publisher is %s", string); 
        
    } else if (xmlStrcasecmp(local, (xmlChar *)"description") == 0) {
      AddNode(meta->description, NewListNode(meta->description, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "description is %s", string);
        
    } else if (xmlStrcasecmp(local, (xmlChar *)"type") == 0) {
      AddNode(meta->type, NewListNode(meta->type, string));       
      _epub_print_debug(opf->epub, DEBUG_INFO, "type is %s", string);
        
    } else if (xmlStrcasecmp(local, (xmlChar *)"format") == 0) {
      AddNode(meta->format, NewListNode(meta->format, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "format is %s", string); 

    } else if (xmlStrcasecmp(local, (xmlChar *)"source") == 0) {
      AddNode(meta->source, NewListNode(meta->source, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "source is %s", string); 

    } else if (xmlStrcasecmp(local, (xmlChar *)"language") == 0) {
      AddNode(meta->lang, NewListNode(meta->lang, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "language is %s", string); 
      
    } else if (xmlStrcasecmp(local, (xmlChar *)"relation") == 0) {
      AddNode(meta->relation, NewListNode(meta->relation, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "relation is %s", string); 

    } else if (xmlStrcasecmp(local, (xmlChar *)"coverage") == 0) {
      AddNode(meta->coverage, NewListNode(meta->coverage, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "coverage is %s", string); 
    } else if (xmlStrcasecmp(local, (xmlChar *)"rights") == 0) {
      AddNode(meta->rights, NewListNode(meta->rights, string));
      _epub_print_debug(opf->epub, DEBUG_INFO, "rights is %s", string);
    } else if (string) {
      if (xmlStrcasecmp(local, (xmlChar *)"dc-metadata") != 0 &&
          xmlStrcasecmp(local, (xmlChar *)"x-metadata") != 0)
        _epub_print_debug(opf->epub, DEBUG_INFO,
                          "unsupported local %s: %s", local, string); 
      free(string);
    }

    ret = xmlTextReaderRead(reader);
  }
}

struct toc *_opf_init_toc() {
  
  struct toc *toc = malloc(sizeof(struct toc));
  
  if (! toc)
    return NULL;
 
  toc->navMap = NULL;
  toc->pageList = NULL;
  toc->navList = NULL;
  toc->playOrder = NewListAlloc(LIST, NULL, NULL, 
                                (NodeCompareFunc)_list_cmp_toc_by_playorder);

  return toc;
}

struct tocCategory *_opf_init_toc_category() {
  struct tocCategory *tc = malloc(sizeof(struct tocCategory));

  tc->id = NULL;
  tc->class = NULL;

  tc->info = NewListAlloc(LIST, NULL, NULL, NULL); //tocLabel
  tc->label = NewListAlloc(LIST, NULL, NULL, NULL); //tocLabel
  tc->items = NewListAlloc(LIST, NULL, NULL, NULL); //tocItem

  return tc;
}

void _opf_free_toc_category(struct tocCategory *tc) {
  if (tc->id)
    free(tc->id);
  if (tc->class)
    free(tc->class);
  
  FreeList(tc->info, (ListFreeFunc)_list_free_toc_label);
  FreeList(tc->label, (ListFreeFunc)_list_free_toc_label);
  FreeList(tc->items, (ListFreeFunc)_list_free_toc_item);
  
  free(tc);
}

void _opf_free_toc(struct toc *toc) {
  
  if (toc->navMap)
    _opf_free_toc_category(toc->navMap);
  if (toc->navList)
    _opf_free_toc_category(toc->navList);
  if (toc->pageList)
    _opf_free_toc_category(toc->pageList);

  // all items are already free, lets free only the struct
  FreeList(toc->playOrder, NULL);

  free(toc);

}

// Parse a navLabel or navInfo returns NULL on failure and the label on success 
struct tocLabel *_opf_parse_navlabel(struct opf *opf, xmlTextReaderPtr reader) {
  int ret;
  
  struct tocLabel *new = malloc(sizeof(struct tocLabel));

  new->lang = xmlTextReaderGetAttribute(reader, (xmlChar *)"lang");
  new->dir = xmlTextReaderGetAttribute(reader, (xmlChar *)"dir");

  ret = xmlTextReaderRead(reader);
  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navLabel") &&
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navInfo")) {
    if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"text") &&
        xmlTextReaderNodeType(reader) == 1) {
      new->text = (xmlChar *)xmlTextReaderReadString(reader);
    }
    ret = xmlTextReaderRead(reader);
  }

  if (ret != 1) {
    free(new);
    return NULL;
  }
  _epub_print_debug(opf->epub, DEBUG_INFO, 
                    "parsing label/info %s(%s/%s)",
                    new->text, new->lang, new->dir);
  return new;
}

struct tocItem *_opf_init_toc_item(int depth) {
  struct tocItem *item = malloc(sizeof(struct tocItem));

  item->label = NULL;
  item->type = NULL;
  item->src = NULL;
  item->depth = depth;
  item->id = NULL;
  item->class = NULL;
  item->playOrder = -1;
  item->value = -1;

  return item;
}

int _get_attribute_as_positive_int(xmlTextReaderPtr reader, const xmlChar *name) {
  xmlChar *str = xmlTextReaderGetAttribute(reader, name);
  int ret = -1;

  if (str) { 
    ret = atoi((char *)str);
    free(str);
  } 

  return ret;
}

void _opf_parse_navmap(struct opf *opf, xmlTextReaderPtr reader) {
  int ret;
  int depth = 0;

  struct tocCategory *tc = _opf_init_toc_category();
  struct tocItem *item = NULL;

  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing nav map");

  tc->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");

  ret = xmlTextReaderRead(reader);
  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navMap")) {

    if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navPoint")) {
      if (xmlTextReaderNodeType(reader) == 1) {

        if (item) {
          _epub_print_debug(opf->epub, DEBUG_INFO, 
                            "adding nav point item->%s %s (d:%d,p:%d)", 
                            item->id, item->src, item->depth, item->playOrder);
          AddNode(tc->items, NewListNode(tc->items, item));
          AddNode(opf->toc->playOrder, NewListNode(opf->toc->playOrder, item));
          item = NULL;
        }

        depth++;
        item = _opf_init_toc_item(depth);
        item->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
        item->class = xmlTextReaderGetAttribute(reader, (xmlChar *)"class");
        
        item->playOrder = _get_attribute_as_positive_int(reader, (xmlChar *)"playOrder");
        if (item->playOrder == -1) {
          _epub_print_debug(opf->epub, DEBUG_WARNING, 
                            "- missing play order in nav point element");
        }
     
      } else if (xmlTextReaderNodeType(reader) == 15) {
        if (item) {
          _epub_print_debug(opf->epub, DEBUG_INFO, 
                            "adding nav point item->%s %s (d:%d,p:%d)", 
                            item->id, item->src, item->depth, item->playOrder);
          AddNode(tc->items, NewListNode(tc->items, item)); 
          AddNode(opf->toc->playOrder, NewListNode(opf->toc->playOrder, item));
          item = NULL;
        }
        depth--;
      }
    }
  
    // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }
    
    if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navLabel")) {
      if (item) {
        if (! item->label)
          item->label = NewListAlloc(LIST, NULL, NULL, NULL); //tocLabel
        AddNode(item->label, NewListNode(item->label, 
                                         _opf_parse_navlabel(opf, reader)));
      } else { // Not inside navpoint
        AddNode(tc->label, NewListNode(tc->label, 
                                       _opf_parse_navlabel(opf, reader)));
      }
    } else if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navInfo")) {
        AddNode(tc->info, NewListNode(tc->info, 
                                      _opf_parse_navlabel(opf, reader)));
        if (item)
          _epub_print_debug(opf->epub, DEBUG_WARNING, 
                            "nav info inside nav point element");
    } else 
      if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"content")) {
        if (item)
          item->src = xmlTextReaderGetAttribute(reader, (xmlChar *)"src");
        else
          _epub_print_debug(opf->epub, DEBUG_WARNING, 
                            "content not inside nav point element");  
      }
    
    ret = xmlTextReaderRead(reader);
  }

  opf->toc->navMap = tc;
  _epub_print_debug(opf->epub, DEBUG_INFO, "finished parsing nav map");
}

void _opf_parse_navlist(struct opf *opf, xmlTextReaderPtr reader) {
  int ret;

  struct tocCategory *tc = _opf_init_toc_category();
  struct tocItem *item = NULL;

  tc->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
  tc->class = xmlTextReaderGetAttribute(reader, (xmlChar *)"class");
    
  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing nav list");

  ret = xmlTextReaderRead(reader);
  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navList")) {

    if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navTarget")) {
      if (xmlTextReaderNodeType(reader) == 1) {
        item = _opf_init_toc_item(1);
        item->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
        item->class = xmlTextReaderGetAttribute(reader, (xmlChar *)"class");
        
        item->playOrder = _get_attribute_as_positive_int(reader, (xmlChar *)"playOrder");
        if (item->playOrder == -1) {
          _epub_print_debug(opf->epub, DEBUG_WARNING, 
                            "- missing play order in nav target element");
        }
        item->value = _get_attribute_as_positive_int(reader, (xmlChar *)"value"); 
      } else if (xmlTextReaderNodeType(reader) == 15) {
        if (item) {
          _epub_print_debug(opf->epub, DEBUG_INFO, 
                            "adding nav target item->%s %s (d:%d,p:%d)", 
                            item->id, item->src, item->depth, item->playOrder);
          AddNode(tc->items, NewListNode(tc->items, item)); 
          AddNode(opf->toc->playOrder, NewListNode(opf->toc->playOrder, item));
          item = NULL;
        } else {
          _epub_print_debug(opf->epub, DEBUG_ERROR, "empty item in nav list"); 
        }
      }
    }
  
    
    // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }
    if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navLabel")) {
      if (item) {
        if (! item->label)
          item->label = NewListAlloc(LIST, NULL, NULL, NULL); //tocLabel
        AddNode(item->label, NewListNode(item->label, 
                                         _opf_parse_navlabel(opf, reader)));
      } else { // Not inside navpoint
        AddNode(tc->label, NewListNode(tc->label, 
                                       _opf_parse_navlabel(opf, reader)));
      }
    } else if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navInfo")) {
      AddNode(tc->info, NewListNode(tc->info, 
                                    _opf_parse_navlabel(opf, reader)));
      if (item)
        _epub_print_debug(opf->epub, DEBUG_WARNING, 
                          "nav info inside nav target element");
    } else 
      if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"content")) {
        if (item)
          item->src = xmlTextReaderGetAttribute(reader, (xmlChar *)"src");
        else
          _epub_print_debug(opf->epub, DEBUG_WARNING, 
                            "content not inside nav target element");  
      }

    ret = xmlTextReaderRead(reader);
  }
  
  opf->toc->navList = tc;
  _epub_print_debug(opf->epub, DEBUG_INFO, "finished parsing nav list");
  
}

void _opf_parse_pagelist(struct opf *opf, xmlTextReaderPtr reader) {
  int ret;
  struct tocCategory *tc = _opf_init_toc_category();
  struct tocItem *item = NULL;
  
  tc->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
  tc->class = xmlTextReaderGetAttribute(reader, (xmlChar *)"class");
  
  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing page list");
  
  ret = xmlTextReaderRead(reader);
  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"pageList")) {
    if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"pageTarget")) {
      if (xmlTextReaderNodeType(reader) == 1) {
        item = _opf_init_toc_item(1);
        item->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
        item->class = xmlTextReaderGetAttribute(reader, (xmlChar *)"class");
        item->type  = xmlTextReaderGetAttribute(reader, (xmlChar *)"type");
        
        item->playOrder = _get_attribute_as_positive_int(reader, (xmlChar *)"playOrder");
        if (item->playOrder == -1) {
          _epub_print_debug(opf->epub, DEBUG_WARNING, 
                            "- missing play order in page target element");
        }
        item->value = _get_attribute_as_positive_int(reader, (xmlChar *)"value"); 
      } else if (xmlTextReaderNodeType(reader) == 15) {
        if (item) {
          _epub_print_debug(opf->epub, DEBUG_INFO, 
                            "adding page target item->%s %s (d:%d,p:%d)", 
                            item->id, item->src, item->depth, item->playOrder);
          AddNode(tc->items, NewListNode(tc->items, item)); 
          AddNode(opf->toc->playOrder, NewListNode(opf->toc->playOrder, item));
          item = NULL;
        } else {
          _epub_print_debug(opf->epub, DEBUG_ERROR, "empty item in nav list"); 
        }
      }
    }

     // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }

    if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navLabel")) {
      if (item) {
        if (! item->label)
          item->label = NewListAlloc(LIST, NULL, NULL, NULL); //tocLabel
        AddNode(item->label, NewListNode(item->label, 
                                         _opf_parse_navlabel(opf, reader)));
      } else { // Not inside navpoint
        AddNode(tc->label, NewListNode(tc->label, 
                                       _opf_parse_navlabel(opf, reader)));
      }
    } else if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"navInfo")) {
      AddNode(tc->info, NewListNode(tc->info, 
                                    _opf_parse_navlabel(opf, reader)));
      if (item)
        _epub_print_debug(opf->epub, DEBUG_WARNING, 
                          "nav info inside page target element");
    } else 
      if (! xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"content")) {
        if (item)
          item->src = xmlTextReaderGetAttribute(reader, (xmlChar *)"src");
        else
          _epub_print_debug(opf->epub, DEBUG_WARNING, 
                            "content not inside nav target element");  
      }

    ret = xmlTextReaderRead(reader);
  }
  
  opf->toc->navList = tc;
  _epub_print_debug(opf->epub, DEBUG_INFO, "finished parsing page list");
    
}

void _opf_parse_toc(struct opf *opf, char *tocStr, int size) {

  xmlTextReaderPtr reader;
  int ret;

  _epub_print_debug(opf->epub, DEBUG_INFO, "building toc");
  
  opf->toc = _opf_init_toc();
  
  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing toc");
  
  reader = xmlReaderForMemory(tocStr, size, "TOC", NULL, 0);
  
  if (reader != NULL) {
    ret = xmlTextReaderRead(reader);
    
    while (ret == 1) {
      
      const xmlChar *name = xmlTextReaderConstName(reader);
      if (xmlStrcasecmp(name, (xmlChar *)"navList") == 0)
        _opf_parse_navlist(opf, reader);
      else 
      if (xmlStrcasecmp(name, (xmlChar *)"navMap") == 0)
        _opf_parse_navmap(opf, reader);
      else 
      if (xmlStrcasecmp(name, (xmlChar *)"pageList") == 0)
        _opf_parse_pagelist(opf, reader);

      ret = xmlTextReaderRead(reader);
    }

    xmlFreeTextReader(reader);
    if (ret != 0) {
      _epub_print_debug(opf->epub, DEBUG_ERROR, "failed to parse toc");
    }
  } else {
    _epub_print_debug(opf->epub, DEBUG_ERROR, "unable to open toc reader");
  }

  SortList(opf->toc->playOrder);
  _epub_print_debug(opf->epub, DEBUG_INFO, "finished parsing toc");
}      

void _opf_parse_spine(struct opf *opf, xmlTextReaderPtr reader) {
  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing spine");

  int ret;
  xmlChar *linear;
  
  opf->spine = NewListAlloc(LIST, NULL, NULL, NULL); 
  opf->tocName = xmlTextReaderGetAttribute(reader, (xmlChar *)"toc");
  
  if (opf->tocName) { 
    char *tocStr;
    struct manifest *item;
    int size;

    _epub_print_debug(opf->epub, DEBUG_INFO, "toc is %s", opf->tocName);
    
    item = _opf_manifest_get_by_id(opf, opf->tocName);
    size = _ocf_get_data_file(opf->epub->ocf, (char *)item->href, &tocStr);
    
    if (size <= 0) 
      _epub_print_debug(opf->epub, DEBUG_ERROR, "unable to open toc file");
    else
    _opf_parse_toc(opf, tocStr, size);

    free(tocStr);

  } else {
    _epub_print_debug(opf->epub, DEBUG_WARNING, "toc not found (-)"); 
    opf->toc = NULL;
  }

  
  ret = xmlTextReaderRead(reader);
  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader), (xmlChar *)"spine")) {
  
    // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }

    struct spine *item = malloc(sizeof(struct spine));

    item->idref = xmlTextReaderGetAttribute(reader, (xmlChar *)"idref");
    linear = xmlTextReaderGetAttribute(reader, (xmlChar *)"linear");
    if (linear && xmlStrcasecmp(linear, (xmlChar *)"no") == 0) {
      item->linear = 0;
    } else {
        item->linear = 1;
        opf->linearCount++;
    }

    if(linear)
        free(linear);
    
     AddNode(opf->spine, NewListNode(opf->spine, item));
     
    // decide what to do with non linear items
    _epub_print_debug(opf->epub, DEBUG_INFO, "found item %s", item->idref);
    
    ret = xmlTextReaderRead(reader);
  }
}

void _opf_parse_manifest(struct opf *opf, xmlTextReaderPtr reader) {
  int ret;
  
  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing manifest");

  opf->manifest = NewListAlloc(LIST, NULL, NULL, 
                               (NodeCompareFunc)_list_cmp_manifest_by_id );

  ret = xmlTextReaderRead(reader);

  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"manifest")) {
    struct manifest *item;

    // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }

    item = malloc(sizeof(struct manifest));

    item->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
    item->href = xmlTextReaderGetAttribute(reader, (xmlChar *)"href");
    item->type = xmlTextReaderGetAttribute(reader, (xmlChar *)"media-type");
    item->fallback = xmlTextReaderGetAttribute(reader, (xmlChar *)"fallback");
    item->fbStyle = 
      xmlTextReaderGetAttribute(reader, (xmlChar *)"fallback-style");
    item->nspace = 
      xmlTextReaderGetAttribute(reader, (xmlChar *)"required-namespace");
    item->modules = 
      xmlTextReaderGetAttribute(reader, (xmlChar *)"required-modules");
    
    _epub_print_debug(opf->epub, DEBUG_INFO, 
                      "manifest item %s href %s media-type %s", 
                      item->id, item->href, item->type);

    AddNode(opf->manifest, NewListNode(opf->manifest, item));

    ret = xmlTextReaderRead(reader);
  }
}

struct manifest *_opf_manifest_get_by_id(struct opf *opf, xmlChar* id) {
  struct manifest data;
  data.id = id;
  
  return FindNode(opf->manifest, &data);
  
}

void _opf_parse_guide(struct opf *opf, xmlTextReaderPtr reader) {
  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing guides");

  int ret;
   opf->guide = NewListAlloc(LIST, NULL, NULL, NULL);

  ret = xmlTextReaderRead(reader);
  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"guides")) {

    // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }
    
    struct guide *item = malloc(sizeof(struct guide));
    item->type = xmlTextReaderGetAttribute(reader, (xmlChar *)"type");
    item->title = xmlTextReaderGetAttribute(reader, (xmlChar *)"href");
    item->href = xmlTextReaderGetAttribute(reader, (xmlChar *)"title");

    _epub_print_debug(opf->epub, DEBUG_INFO, 
                      "guide item: %s href: %s type: %s", 
                      item->title, item->href, item->type);
    AddNode(opf->guide, NewListNode(opf->guide, item));
    ret = xmlTextReaderRead(reader);
  }
}

listPtr _opf_parse_tour(struct opf *opf, xmlTextReaderPtr reader) {
  int ret;
  listPtr tour = NewListAlloc(LIST, NULL, NULL, NULL);

  ret = xmlTextReaderRead(reader);
  
  while (ret == 1 && 
         xmlStrcasecmp(xmlTextReaderConstName(reader),(xmlChar *)"tour")) {

    // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }
    
    struct site *item = malloc(sizeof(struct site));
    item->title = xmlTextReaderGetAttribute(reader, (xmlChar *)"title");
    item->href = xmlTextReaderGetAttribute(reader, (xmlChar *)"href");
    AddNode(tour, NewListNode(tour, item));

    ret = xmlTextReaderRead(reader);
  }

  return tour;
}

void _opf_parse_tours(struct opf *opf, xmlTextReaderPtr reader) {
  _epub_print_debug(opf->epub, DEBUG_INFO, "parsing tours");

  int ret;
  opf->tours = NewListAlloc(LIST, NULL, NULL, NULL);

  ret = xmlTextReaderRead(reader);
  xmlChar *local = (xmlChar *)xmlTextReaderConstName(reader);
  
  while (ret == 1 && 
         xmlStrcasecmp(local,(xmlChar *)"tours")) {
    
    
    // ignore non starting tags
    if (xmlTextReaderNodeType(reader) != 1) {
      ret = xmlTextReaderRead(reader);
      continue;
    }
    
    if (xmlStrcasecmp(local, (xmlChar *)"tour") == 0) {
      struct tour *item = malloc(sizeof(struct tour));
      
      item->title = xmlTextReaderGetAttribute(reader, (xmlChar *)"title");
      item->id = xmlTextReaderGetAttribute(reader, (xmlChar *)"id");
      item->sites = _opf_parse_tour(opf, reader);

      AddNode(opf->tours, NewListNode(opf->tours, item));
    }
    
    free(local);
    local = (xmlChar *)xmlTextReaderConstName(reader);
    ret = xmlTextReaderRead(reader);
  }

  free(local);
}

xmlChar *_opf_label_get_by_lang(struct opf *opf, listPtr label, char *lang) {
  struct tocLabel data, *tmp;
  data.lang = (xmlChar *)lang;
  label->compare = (NodeCompareFunc)_list_cmp_label_by_lang;
  tmp =  FindNode(label, &data);
  return (tmp?tmp->text:NULL);
  
}

xmlChar *_opf_label_get_by_doc_lang(struct opf *opf, listPtr label) {
  opf->metadata->lang->Current = opf->metadata->lang->Head;
  return _opf_label_get_by_lang(opf, label, 
                                (char *)GetNode(opf->metadata->lang));
}

void _opf_dump(struct opf *opf) {
  printf("Title(s):\n   ");
  DumpList(opf->metadata->title, (ListDumpFunc)_list_dump_string);
  printf("Creator(s):\n   ");
  DumpList(opf->metadata->creator, (ListDumpFunc)_list_dump_creator);
  printf("Identifier(s):\n   ");
  DumpList(opf->metadata->id, (ListDumpFunc)_list_dump_id);
  printf("Reading order:\n   ");
  DumpList(opf->spine, (ListDumpFunc)_list_dump_spine);
  printf("\n");
  if (opf->guide) {
    printf("Guide:\n");
    DumpList(opf->guide, (ListDumpFunc)_list_dump_guide);
  }
  if (opf->tours)
    DumpList(opf->tours, (ListDumpFunc)_list_dump_tour);
  if (opf->metadata->meta->Size != 0) {
    printf("Extra local metadata:\n");
    DumpList(opf->metadata->meta, (ListDumpFunc)_list_dump_meta);
  }
}

void _opf_close(struct opf *opf) {
  if (opf->metadata)
    _opf_free_metadata(opf->metadata);
  if (opf->toc)
    _opf_free_toc(opf->toc);
  if (opf->spine)
    FreeList(opf->spine, (ListFreeFunc)_list_free_spine);
  if (opf->tocName)
    free(opf->tocName);
  if (opf->manifest)
    FreeList(opf->manifest, (ListFreeFunc)_list_free_manifest);
  if (opf->guide)
    FreeList(opf->guide, (ListFreeFunc)_list_free_guide);
  if (opf->tours)
    FreeList(opf->tours, (ListFreeFunc)_list_free_tours);
  free(opf);
}
