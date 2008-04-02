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
    fprintf(stderr, "   -v\t Verbose (error)\n");
    fprintf(stderr, "   -vv\t Verbose (warnings)\n");
    fprintf(stderr, "   -vvv\t Verbose (info)\n");
    fprintf(stderr, "   -d\t Debug mode (implies -vvv)\n");
    fprintf(stderr, "   -p\t Linear print book (normal reading)\n");
    fprintf(stderr, "   -pp\t Print the whole book\n");
    fprintf(stderr, "   -t <tour id>\t prints the tour <tour id>\n");
    fprintf(stderr, "   -g Print the guide file as html (if exists)\n");
    fprintf(stderr, "   -n Print the toc ncx file as html (if exists)\n");

    exit(code);
}

int main(int argc , char **argv) {
  struct epub *epub;
  char *filename = NULL;
  char *tourId = NULL;
  int verbose = 0, print = 0, debug = 0, ncx = 0, guide = 0;
  
  if (argc < 2) {
      usage(3);
 }

  int i, j;
  
  for (i = 1;i<argc;i++) {
      if (argv[i][0] == '-') {

        loop:          
          for (j = 1;j<strlen(argv[i]);j++) {
              switch(argv[i][j]) {
                case 'v':
                    verbose++;
                    break;
                case 'd':
                    debug++;
                    break;
                case 'p':
                    print++;
                    break;
                case 'h':
                    usage(0);
                    break;
                case 'g':
                    guide++;
                    break;
                case 'n': 
                    ncx++;
                    break;
                case 't':
                    i++;
                    tourId = argv[i];
                    goto loop;
                    break;
                default:
                    usage(2);
                    break;
              }
          }

      } else {
          if (! filename) {
              filename = argv[i];
          } else {
              usage(1);
          }
      }
  }
  
  if (debug)
      verbose = 4;
  
  if (! (epub = epub_open(filename, verbose)))
    quit(1);
  
  epub_dump(epub);


  if (! epub_close(epub)) {
    quit(1);
  }

  quit(0);
  return 0;
}
