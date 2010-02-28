#ifndef EPUB_H
#define EPUB_H 1

#include <epub_shared.h>
/** \struct epub is a private struct containting information about the epub file */
struct epub;

/** \struct eiterator is a private iterator struct */
struct eiterator;
struct titerator;

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

  /** 
      This function accepts an epub filename. It then parses its information and
      returns it as an epub struct.
      
      @param filename the name of the file to open
      @param debug is the debug level (0=none, 1=errors, 2=warnings, 3=info)
      @return epub struct with the information of the file or NULL on error
      
  */
  EPUB_EXPORT struct epub *epub_open(const char *filename, int debug);
  
  /**
     This function sets the debug level to the given level.
     
     @param filename is the name of the file to open
     @param debug is the debug level (0=none, 1=errors, 2=warnings, 3=info)
  */
  EPUB_EXPORT void epub_set_debug(struct epub *epub, int debug);

  /** 
      returns the file with the give filename

      @param epub struct of the epub file we want to read from
      @param filename the name of the file we want to read
      @param pointer to where the file data is stored
      @return the number of bytes read
  */
  EPUB_EXPORT int epub_get_ocf_file(struct epub *epub, const char *filename, char **data);
  
  /** 
      Frees the memory held by the given iterator
      
      @param it the iterator
  */
  EPUB_EXPORT void epub_free_iterator(struct eiterator *it);

  /** 
      This function closes a given epub. It also frees the epub struct.
      So you can use it after calling this function.
      
      @param epub the struct of the epub to close.
  */
  EPUB_EXPORT int epub_close(struct epub *epub);
  
  /** 
      Debugging function dumping various file information.
      
      @param epub the struct of the epub to close.
  */
  EPUB_EXPORT void epub_dump(struct epub *epub);
  
    
  /** 
      (Bad xml might cause some of it to be NULL).

      @param epub the struct .
  */
  EPUB_EXPORT unsigned char **epub_get_metadata(struct epub *epub, enum epub_metadata type,
                                                int *size);

  /** 
      returns the file with the give filename. The file is looked
      for in the data directory. (Useful for getting book files). 

      @param epub struct of the epub file we want to read from
      @param filename the name of the file we want to read
      @param pointer to where the file data is stored
      @return the number of bytes read
  */
  EPUB_EXPORT int epub_get_data(struct epub *epub, const char *name, char **data);

  
  /** 
      Returns a book iterator of the requested type
      for the given epub struct.
      
      @param epub struct of the epub file
      @param type the iterator type
      @param opt other options (ignored for now)
      @return eiterator to the epub book
  */
  EPUB_EXPORT struct eiterator *epub_get_iterator(struct epub *epub, 
                                                  enum eiterator_type type, int opt);

  /**
     updates the iterator to the next element and returns a pointer 
     to the data. the iterator handles the freeing of the memory.
     
     @param it the iterator
     @return pointer to the data
  */
  EPUB_EXPORT char *epub_it_get_next(struct eiterator *it);

  /**
     Returns a pointer to the iterator's data. the iterator handles 
     the freeing of the memory.
     
     @param it the iterator
     @return pointer to the data
  */
  EPUB_EXPORT char *epub_it_get_curr(struct eiterator *it);
  
  /**
     Returns a pointer to the url of the iterator's current data. 
     the iterator handles the freeing of the memory.
     
     @param it the iterator
     @return pointer to the current data's url
  */
  EPUB_EXPORT char *epub_it_get_curr_url(struct eiterator *it);

  /** 
      Returns a book toc iterator of the requested type
      for the given epub struct.
      
      @param epub struct of the epub file
      @param type the iterator type
      @param opt other options (ignored for now)
      @return toc iterator to the epub book
  */
  EPUB_EXPORT struct titerator *epub_get_titerator(struct epub *epub, 
                                                   enum titerator_type type, int opt);

  
  /**
     Returns 1 if the current entry is valid and 0 otherwise. 

     @param tit the iterator
     @return 1 if the current entry is valid and 0 otherwise 
  */
  EPUB_EXPORT int epub_tit_curr_valid(struct titerator *tit);
  
  /**
     Returns a pointer to the depth of the toc iterator's current entry. 
     the iterator handles the freeing of the memory.
     
     @param tit the iterator
     @return pointer to the current entry's depth
  */
  EPUB_EXPORT int epub_tit_get_curr_depth(struct titerator *tit);

  /**
     Returns a pointer to the link of the toc iterator's current entry. 
     the iterator handles the freeing of the memory.
     
     @param tit the iterator
     @return the current entry's depth
  */
  EPUB_EXPORT char *epub_tit_get_curr_link(struct titerator *tit);

  /**
     Returns a pointer to the label of the toc iterator's current entry. 
     the iterator handles the freeing of the memory.
     
     @param tit the iterator
     @return pointer to the current entry's lable
  */
  EPUB_EXPORT char *epub_tit_get_curr_label(struct titerator *tit);

  /** 
      Frees the memory held by the given iterator
      
      @param tit the iterator
  */
  EPUB_EXPORT void epub_free_titerator(struct titerator *tit);
  
  /**
     updates the iterator to the next element.
     
     @param tit the iterator
     @return 1 on success and 0 otherwise
  */
  EPUB_EXPORT int epub_tit_next(struct titerator *tit);

  /**
     Cleans up after the library. Call this when you are done with the library. 
  */
  EPUB_EXPORT void epub_cleanup();

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* EPUB_H */
