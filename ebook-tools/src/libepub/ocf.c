#include "epublib.h"

int _ocf_parse_mimetype(struct ocf *ocf) {

  _epub_print_debug(ocf->epub, DEBUG_INFO, "looking for mime type");
  // Figure out mime type
  if (_ocf_get_file(ocf, MIMETYPE_FILENAME, &ocf->mimetype) == -1) {
    _epub_print_debug(ocf->epub, DEBUG_WARNING, 
                      "Can't get mimetype, assuming application/epub+zip (-)");
    ocf->mimetype = malloc(sizeof(char) * strlen("application/epub+zip")+1);
    strcpy(ocf->mimetype, "application/epub+zip");
  } else {
    _epub_print_debug(ocf->epub, DEBUG_INFO, "mimetype found %s", ocf->mimetype);
  }

  return 1;
}

int _ocf_parse_container(struct ocf *ocf) {

  _epub_print_debug(ocf->epub, DEBUG_INFO, "parsing container file %s", 
                    METAINFO_DIR "/" CONTAINER_FILENAME);

  char *containerXml = NULL;
  char *name = CONTAINER_FILENAME;
  if (! _ocf_get_file(ocf, METAINFO_DIR "/" CONTAINER_FILENAME, &containerXml))
    return 0;
  
  xmlTextReaderPtr reader;
  int ret;

  reader = xmlReaderForMemory(containerXml, strlen(containerXml), 
                              name, NULL, 0);
  if (reader != NULL) {
    ret = xmlTextReaderRead(reader);

	while (ret == 1) {
		// Checking for container tag for validation
		if(xmlStrcasecmp(xmlTextReaderConstLocalName(reader),
						 (xmlChar *)"container") == 0) {
			
			_epub_print_debug(ocf->epub, DEBUG_INFO, 
							  "Found containerr");
		// Checking for rootfiles tag for validation
		} else if(xmlStrcasecmp(xmlTextReaderConstLocalName(reader),
								(xmlChar *)"rootfiles") == 0) {
			
			_epub_print_debug(ocf->epub, DEBUG_INFO, 
							  "Found rootfiles");
		} else if (xmlStrcasecmp(xmlTextReaderConstLocalName(reader),
								 (xmlChar *)"rootfile") == 0) {
				
			struct root *newroot = malloc(sizeof(struct root));
			newroot->mediatype = 
				xmlTextReaderGetAttribute(reader, (xmlChar *)"media-type");
			newroot->fullpath =
				xmlTextReaderGetAttribute(reader, (xmlChar *)"full-path");
			AddNode(ocf->roots, NewListNode(ocf->roots, newroot));
			
				_epub_print_debug(ocf->epub, DEBUG_INFO, 
								  "found root in %s media-type is %s",
								  newroot->fullpath, newroot->mediatype);
				
		}
		ret = xmlTextReaderRead(reader);
	}
	
    xmlFreeTextReader(reader);
    free(containerXml);
    if (ret != 0) {
      _epub_print_debug(ocf->epub, DEBUG_ERROR, "failed to parse %s\n", name);
      return 0;
    }
  } else {
    _epub_print_debug(ocf->epub, DEBUG_ERROR, "unable to open %s\n", name);
    return 0;
  }
  
  
  return 1;
}

void _ocf_dump(struct ocf *ocf) {  
  printf("Filename:\t %s\n", ocf->filename);

  printf("Root(s):\n");
  DumpList(ocf->roots, (ListDumpFunc)_list_dump_root);

}

struct zip *_ocf_open(struct ocf *ocf, const char *filename) {

  int err;
  char errStr[8192];
  struct zip *arch = NULL;

  if (! (arch = zip_open(filename, 0, &err))) {
    zip_error_to_str(errStr, sizeof(errStr), err, errno);
    _epub_print_debug(ocf->epub, DEBUG_ERROR, "%s - %s", filename, errStr); 
  }
  
  return arch;
}

void _ocf_close(struct ocf *ocf) {

  if (ocf->arch) {
    if (zip_close(ocf->arch) == -1) {
      _epub_print_debug(ocf->epub, DEBUG_ERROR, "%s - %s\n", 
                        ocf->filename, zip_strerror(ocf->arch));
    }
  }
  
  FreeList(ocf->roots, (ListFreeFunc)_list_free_root);

  if (ocf->filename)
    free(ocf->filename);
  if (ocf->mimetype)
    free(ocf->mimetype);
  if (ocf->datapath)
    free(ocf->datapath);
  free(ocf);
  
}

// returns index if file exists else -1
int _ocf_check_file(struct ocf *ocf, const char *filename) {
  return zip_name_locate(ocf->arch, filename, 0);
}

// Get the file named filename from epub zip and pub it in fileStr
// Returns the size of the file or -1 on failure
int _ocf_get_file(struct ocf *ocf, const char *filename, char **fileStr) {
  
  struct epub *epub = ocf->epub;
  struct zip *arch = ocf->arch;
  
  struct zip_file *file = NULL;
  struct zip_stat *fileStat = malloc(sizeof(struct zip_stat));

  *fileStr = NULL;

  if (zip_stat(arch, filename, ZIP_FL_UNCHANGED, fileStat) == -1) {
    _epub_print_debug(epub, DEBUG_INFO, "%s - %s", 
                      filename, zip_strerror(arch));
    free(fileStat);
    return -1;
  }

  if (! (file = zip_fopen_index(arch, fileStat->index, ZIP_FL_NODIR))) {
    _epub_print_debug(epub, DEBUG_INFO, "%s - %s", 
                      filename, zip_strerror(arch));
    free(fileStat);
    return -1;
  }

  *fileStr = (char *)malloc((fileStat->size+1)* sizeof(char));
  
  int size;
  if ((size = zip_fread(file, *fileStr, fileStat->size)) == -1) {
    _epub_print_debug(epub, DEBUG_INFO, "%s - %s", 
                      filename, zip_strerror(arch));
  } else {
    (*fileStr)[size] = 0;
  }
  
  free(fileStat);

  if (zip_fclose(file) == -1) {
    _epub_print_debug(epub, DEBUG_INFO, "%s - %s", 
                      filename, zip_strerror(arch));
    free(*fileStr);
    *fileStr = NULL;
    free(fileStat);
    return -1;
  }
  
  if (epub->debug >= DEBUG_VERBOSE) {
    _epub_print_debug(epub, DEBUG_VERBOSE, "--------- Begin %s", filename);
    fprintf(stderr, "%s\n", (*fileStr));
    _epub_print_debug(epub, DEBUG_VERBOSE, "--------- End %s", filename);
  }
  return size;
}


void _ocf_not_supported(struct ocf *ocf, const char *filename) {
  if (_ocf_check_file(ocf, filename) > -1) 
    _epub_print_debug(ocf->epub, DEBUG_WARNING, 
                      "file %s exists but is not supported by this version", filename);
}

struct ocf *_ocf_parse(struct epub *epub, const char *filename) {
  _epub_print_debug(epub, DEBUG_INFO, "building ocf struct");
  
  struct ocf *ocf = malloc(sizeof(struct ocf));
  ocf->epub = epub;
  ocf->datapath = NULL;
  ocf->roots = NewListAlloc(LIST, NULL, NULL, 
                            (NodeCompareFunc)_list_cmp_root_by_mediatype);
  ocf->filename = malloc(sizeof(char)*(strlen(filename)+1));

  strcpy(ocf->filename, filename);
  
  if (! (ocf->arch = _ocf_open(ocf, ocf->filename)))
    return NULL;
  
  // Find the mime type
  _ocf_parse_mimetype(ocf);

  // Parse the container for roots
  _ocf_parse_container(ocf);
    
  // Unsupported files
   _ocf_not_supported(ocf, METAINFO_DIR "/" MANIFEST_FILENAME);
   _ocf_not_supported(ocf, METAINFO_DIR "/" METADATA_FILENAME);
   _ocf_not_supported(ocf, METAINFO_DIR "/" SIGNATURES_FILENAME);
   _ocf_not_supported(ocf, METAINFO_DIR "/" ENCRYPTION_FILENAME);
   _ocf_not_supported(ocf, METAINFO_DIR "/" RIGHTS_FILENAME);

  return ocf;
}

int _ocf_get_data_file(struct ocf *ocf, const char *filename, char **fileStr) {
  int size;
  char *fullname = malloc((strlen(filename)+strlen(ocf->datapath)+1)*sizeof(char));
  strcpy(fullname, ocf->datapath);
  strcat(fullname, filename);
  size = _ocf_get_file(ocf, fullname, fileStr);
  free(fullname);

  return size;
}

char *_ocf_root_fullpath_by_type(struct ocf *ocf, char *type) {
  struct root look = {(xmlChar *)type, NULL};
  struct root *res;
  
  res = FindNode(ocf->roots, &look);
  if (res)
    return strdup((char *)res->fullpath);
  
  _epub_print_debug(ocf->epub, DEBUG_WARNING, 
                      "type %s for root not found", type);
  return NULL;
    
    
}

char *_ocf_root_by_type(struct ocf *ocf, char *type) {
  struct root look = {(xmlChar *)type, NULL};
  struct root *res;
  char *rootXml = NULL;

  res = FindNode(ocf->roots, &look);
  if (res)
    _ocf_get_file(ocf, (char *)res->fullpath, &rootXml);


  if (! rootXml)
    _epub_print_debug(ocf->epub, DEBUG_WARNING, 
                      "type %s for root not found", type);

  return rootXml;
}

