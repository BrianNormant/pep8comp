#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "parser.h"

#define FILENAME argv[1]

/* TODO list
   - pep 8 compiler
   [ ] Support for comment
   [ ] Support for escape sequences in string
   [ ] Support for escape sequences in characters
   [x] identify labels
   [ ] convert intruction to bytecode and generate assembly code
   
   - pep 8 emulator
   [ ] register
   [ ] basic arithmetic addr subr aslr ect...
   [ ] basic I/O
   [ ] memory read and write
   [ ] jumps
   [ ] calls
   [ ] addressing modes
   [ ] stack
   Special
   [ ] Bitshift instructions
   [ ] Floating point support
   [ ] 
*/
int main(int argc, char *argv[]) {
        // Check correct usage
        switch (argc) {
                case 1:
                        printf("Usage: %s <file>\n", argv[0]);
                        break;
                case 2: break;
                default:
                        printf("Usage: %s <file>\n", argv[0]);
                        return EXIT_FAILURE;
        }
        // Open file
        FILE *f;
        if (!(f = fopen(FILENAME, "r"))) {
                perror("fopen");
                fprintf(stderr, "Error openning %s\n", FILENAME);
                return EXIT_FAILURE;
        }
        // Parse file syntax
        parse_init(f);
        if (!parse()) return EXIT_FAILURE;
        parse_deinit();

        // Close file
        fclose(f);

        return EXIT_SUCCESS;
}
