#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <epub.h>

void quit(int code) {
  epub_cleanup();
  exit(code);
}

void usage(int code) {
  fprintf(stderr, "Usage: einfo [options] <filename>\n");
  fprintf(stderr, "   -h\t Help message\n");
  fprintf(stderr, "   -q\t don't print short meta info summary\n");
  fprintf(stderr, "   -v\t Verbose (error)\n");
  fprintf(stderr, "   -vv\t Verbose (warnings)\n");
  fprintf(stderr, "   -vvv\t Verbose (info)\n");
  fprintf(stderr, "   -d\t Debug mode (implies -vvv)\n");
  fprintf(stderr, "   -p\t Linear print book (normal reading)\n");
  fprintf(stderr, "   -pp\t Print the whole book\n");
  fprintf(stderr, "   -t <tour id>\t prints the tour <tour id>\n");

  exit(code);
}

int main(int argc , char **argv) {
  struct epub *epub;
  char *filename = NULL;
  char *tourId = NULL;
  int verbose = 0, print = 0, debug = 0, quiet = 0, tour = 0;
  
  int i, j;
  
  for (i = 1;i<argc;i++) {
    loop:          

    if (argv[i][0] == '-') {

      for (j = 1;j<strlen(argv[i]);j++) {
        switch(argv[i][j]) {
        case 'v':
          verbose++;
          break;
        case 'd':
          debug++;
          break;
        case 'q':
          quiet++;
          break;
        case 'p':
          print++;
          break;
        case 'h':
          usage(0);
          break;
        case 't':
          tour++;
          if (tour > 1) {
            fprintf(stderr, "Only one tour at a time is supported\n");
            usage(2);
          }

          i++;
          if (i<argc) {
            tourId = argv[i];
          } else {  
            fprintf(stderr, "Missing tour id\n");
            usage(2);
          }

          i++;
          goto loop;
          break;
        default:
          fprintf(stderr, "Unknown flag %s\n", argv[i]);
          usage(2);
          break;
        }
      }

    } else {
      if (! filename) {
        filename = argv[i];
      } else {
        fprintf(stderr, "Missing file name\n");
        usage(1);
      }
    }
  }
         
  if (! filename) {
    fprintf(stderr, "Missing file name\n");
    usage(1);
  }
  
  if (debug)
    verbose = 4;
  
  if (! (epub = epub_open(filename, verbose)))
    quit(1);
  
  if (! quiet)
    epub_dump(epub);

  // Print the book
  if (print > 0) {

    struct eiterator *it;

    if (print > 1) {
      it = epub_get_iterator(epub, EITERATOR_LINEAR, 0);   
    } else {
      it = epub_get_iterator(epub, EITERATOR_SPINE, 0);
    }
    
    do {
      if (epub_it_get_curr(it)) {
      
        printf("%s\n", epub_it_get_curr(it));
      }
    } while (epub_it_get_next(it));

    epub_free_iterator(it);

  }
  
  if (tour) {
    printf("Tours are still not supported\n");
  }

  if (! epub_close(epub)) {
    quit(1);
  }

  quit(0);
  return 0;
}
