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
   [x] Support for comment
   [ ] Support for escape sequences in string
   [ ] Support for escape sequences in characters
   [ ] Support for instruction ret2 ret3, ect
   [ ] Support for more escape sequences (currently n,r,t,x)
   [ ] Option for raw string literals (without null terminator)
   [x] identify labels
   [t] convert .dot command to bytecode
   [ ] convert intruction to bytecode and generate assembly code  

   - pep 8 emulator
   [ ] registers
   [ ] I/O
   [ ] arithmetic addr, subr, aslr, ...
   [ ] control flow (BR, BREQ, ...)
   [ ] memory read and write
   [ ] jumps
   [ ] calls
   [ ] addressing modes
   [ ] stack
   
   Special
   ; extend the simulated cpu to 32 to support more instructions
*/

static char program[500]; // FIXME
static uint16_t program_size;
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
        print_parsed();
        if (!parse(program, &program_size)) return EXIT_FAILURE;
        printf_bytes(program, program_size);
        parse_deinit();

        // Close file
        fclose(f);

        return EXIT_SUCCESS;
}
