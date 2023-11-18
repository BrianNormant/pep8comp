#include <bits/types/sigevent_t.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "sugar.h"
#include "parser.h"

static int _initialized = 0;

static char source_lines[MAXLINES][MAXCOLS]; // Storage for lines
static int lines_cnt = 0;

static struct label_info source_labels[MAXLBL];
static int label_cnt = 0;

static pcre2_code *line_re;
static pcre2_match_data *line_matchd;
static pcre2_code *arg_re;
static pcre2_match_data *arg_matchd;
static pcre2_code *str_re;
static pcre2_match_data *str_matchd;
static pcre2_code *size_re;
static pcre2_match_data *size_matchd;
static int errornumber;
static unsigned long erroroffset;

int // compile regexes, read file
parse_init(FILE* file)
{ 
        // compile line pattern
        printf("Line regex is: %s\n", LINE_PATTERN);
        line_re = pcre2_compile(
                LINE_PATTERN,
                PCRE2_ZERO_TERMINATED,
                0,
                &errornumber,
                &erroroffset,
                NULL);
        if (!line_re) {
                perror("Failed to compile line pattern");
                return 1;
        }
        line_matchd = pcre2_match_data_create_from_pattern(line_re, NULL);
        arg_re = pcre2_compile(
                ARG_PATTERN,
                PCRE2_ZERO_TERMINATED,
                0,
                &errornumber,
                &erroroffset,
                NULL);
        if (!arg_re) {
                perror("Failed to compile arg pattern");
                return 1;
        }
        arg_matchd = pcre2_match_data_create_from_pattern(arg_re, NULL);

        str_re = pcre2_compile(
                STR_PATTERN,
                PCRE2_ZERO_TERMINATED,
                0,
                &errornumber,
                &erroroffset,
                NULL);
        if (!str_re) {
                perror("Failed to compile str pattern");
                return 1;
        }
        str_matchd = pcre2_match_data_create_from_pattern(str_re, NULL);
        size_re = pcre2_compile(
                ARG_PATTERN,
                PCRE2_ZERO_TERMINATED,
                0,
                &errornumber,
                &erroroffset,
                NULL);
        if (!size_re) {
                perror("Failed to compile size pattern");
                return 1;
        }
        size_matchd = pcre2_match_data_create_from_pattern(size_re, NULL);
        // Read all lines from the file and store them in line
        int line = 0,
            col = 0,
            quote = 0,
            escape = 0,
            wait_for_nl = 0;
        while (1) {
                if (line >= MAXLINES) {
                        perror("Too many lines in file");
                        return 0;
                }

                if (col >= MAXCOLS) {
                        fprintf(stderr, "Too many columns in line %d\n", line);
                        return 0;
                }
                char c = fgetc(file);
                if (c == EOF) break;
                if (wait_for_nl) {
                        if (c == '\n' || c == '\r') {
                                wait_for_nl = 0;
                                continue;
                        } else continue;
                }
                if (c == '\n' || c == '\r') {
                        line++;
                        source_lines[line][col] = '\0';
                        col = quote = escape = 0;
                        continue;
                }
                if (c == ';' && !quote) {
                        line++;
                        col = quote = escape = 0;
                        source_lines[line][col] = '\0';
                        wait_for_nl = 1;
                        continue;
                }
                if (c == '"') {
                        if (quote && !escape) {
                                quote = 0; // End of string
                        } else {
                                quote = 1; // Either first quote or escaped quote 
                        }
                }
                if (escape) { // The character has been escaped so reset escape
                        escape = 0;
                } else if (c == '\\') {
                        escape = 1;
                }
                source_lines[line][col] = c;
                col++;
        }
        lines_cnt = line;
        _initialized = 1;
        return 1;
}

void // Print all parsed lines
print_parsed() 
{
        assert(_initialized);
        FORI(i, 0, lines_cnt) printf("%s\n", source_lines[i]);
}

void // free mem and exit
parser_failure() 
{
        pcre2_match_data_free(line_matchd);
        pcre2_code_free(line_re);
        pcre2_match_data_free(arg_matchd);
        pcre2_code_free(arg_re);
        pcre2_match_data_free(str_matchd);
        pcre2_code_free(str_re);
        pcre2_match_data_free(size_matchd);
        pcre2_code_free(size_re);
}

int
match_line(PCRE2_SPTR subject, int line, int *rc, size_t **ov)
{
        *rc = pcre2_match(
                line_re,
                subject,
                strlen((char*)subject),
                0,
                0,
                line_matchd,
                NULL);
        if (*rc < 0) { 
                fprintf(stderr,"pcre2_match failed at line %d\n", line);
                return 0;
        }
        if (!*rc) {
                perror("ovector");
                return 0;
        }
        *ov = pcre2_get_ovector_pointer(line_matchd);
        return 1;
}

int
match_arg(PCRE2_SPTR subject, int line, int *rc, size_t **ov) {
        *rc = pcre2_match(
                arg_re,
                subject,
                strlen((char*)subject),
                0,
                0,
                arg_matchd,
                NULL);
        if (*rc < 0) { 
                fprintf(stderr,"pcre2_match failed at line %d\n", line);
                return 0;
        }
        if (!*rc) {
                perror("ovector");
                return 0;
        }
        *ov = pcre2_get_ovector_pointer(arg_matchd);
        return 1;
}

int
match_str(PCRE2_SPTR subject, size_t len, int line, int *rc, size_t **ov) {
        *rc = pcre2_match(
                str_re,
                subject,
                len,
                0,
                0,
                str_matchd,
                NULL);
        if (*rc < 0) { 
                fprintf(stderr,"Failed to match string at line %d\n", line);
                return 0;
        }
        if (!*rc) {
                perror("ovector");
                return 0;
        }
        *ov = pcre2_get_ovector_pointer(str_matchd);
        return 1;
}

int
match_size(PCRE2_SPTR subject, int line, int *rc, size_t **ov) {
        *rc = pcre2_match(
                size_re,
                subject,
                strlen((char*)subject),
                0,
                0,
                size_matchd,
                NULL);
        if (*rc < 0) { 
                fprintf(stderr,"Failed to match size for .BLOCK at line %d\n", line);
                return 0;
        }
        if (!*rc) {
                perror("ovector");
                return 0;
        }
        *ov = pcre2_get_ovector_pointer(size_matchd);
        return 1;
}

int
add_label(PCRE2_SPTR id, size_t len, size_t pos) {
        if (label_cnt >= MAXLBL) {
                perror("Too many labels");
                return 0;
        }
        source_labels[label_cnt].id = (char*)id;
        source_labels[label_cnt].id_len = len;
        source_labels[label_cnt].pos = pos;
        printf("Found Label \"%.*s\" at position %zu\n",
                        (int)source_labels[label_cnt].id_len,
                        source_labels[label_cnt].id,
                        source_labels[label_cnt].pos);
        label_cnt++;
        return 1;
}

size_t 
instr_byte_size(PCRE2_SPTR instr, size_t instr_len,
                PCRE2_SPTR arg, size_t arg_len) 
{
        // .BYTE: add 1 to bytecode_pos
        // .WORD: add 2 to bytecode_pos
        // .ADDRESS : add 2 to bytecode_pos
        // .ASCII : add len of string to bytecode_pos
        // .BLOCK: add arg to bytecode_pos
        // Otherwise add 3 to bytecode_pos
        if (!instr_len) return 0;
        printf("  instr: [%.*s]\n", (int)instr_len, instr);
        printf("  arg: [%.*s]\n", (int)arg_len, arg);
        if (instr[0] == '.') {
                if (strncmp((char*)instr, ".BYTE", 5) == 0)
                        return 1;
                else if (strncmp((char*)instr, ".WORD", 5) == 0)
                        return 2;
                else if (strncmp((char*)instr, ".ADDRESS", 8) == 0)
                        return 2;
                else if (strncmp((char*)instr, ".ASCII", 6) == 0) {
                        int rc;
                        size_t *ov;
                        if (!match_str(arg, arg_len, 0, &rc, &ov))
                                return -1;
                        // char *str = arg + ov[2*1];
                        int str_len = ov[2*1+1] - ov[2*1];
                        printf("  Size of string is %d\n", str_len);
                        // TODO handle escape characters
                        // Parse again with a special regex
                        return str_len;
                } else if (strncmp((char*)instr, ".BLOCK", 6) == 0) {
                        int rc;
                        size_t *ov;
                        if (!match_size(arg, 0, &rc, &ov))
                                return -1;
                        char *size_c = (char*)arg + ov[2*1];
                        int size;
                        sscanf(size_c, "%d", &size);
                        printf("  Size of block is %d\n", size);
                        return size;
                } else return 3;
        } else {
                return 3;
        }
}

void
print_match(PCRE2_SPTR subject, int rc, size_t *ov) {
        FORI(i, 0, rc) {
                PCRE2_SPTR sub = subject + ov[2*i];
                int sub_size = ov[2*i+1] - ov[2*i];
                printf("\tgroup %d: [%.*s]\n", i, sub_size, sub);
        }
}

/*
 *Parse line by line
 *First find labels and store their respective positions
 *Then parse Instruction and block command to determine the bytecode
*/
int
parse()
{
        assert(_initialized);
        
        size_t *ovector;
        int rc;
        // first pass, identify labels and their positions in the final bytecode
        size_t bytecode_pos = 0;
        FORI(i, 0, lines_cnt) {
                PCRE2_SPTR line = (PCRE2_SPTR)source_lines[i];
                printf("Matching \"%s\"\n", line);
                if (!match_line(line, i, &rc, &ovector)) {
                        parser_failure();
                        return 0;
                }
                PCRE2_SPTR match = line + ovector[2*0];
                size_t match_len = ovector[2*0+1] - ovector[2*0];
                printf("Matched \"%.*s\"\n", (int)match_len, match);
                print_match(line, rc, ovector);

                // Label declaration is first identifier group
                PCRE2_SPTR label_id = line + ovector[2*1];
                int label_id_len = ovector[2*1 + 1] - ovector[2*1];

                DLOG(label_id_len);
                if (label_id_len)
                        if (!add_label(label_id, label_id_len-1, bytecode_pos)) {
                                parser_failure();
                                return 0;
                        }

                PCRE2_SPTR instr = line + ovector[2*2];
                int instr_len = ovector[2*2 + 1] - ovector[2*2];
                PCRE2_SPTR arg = line + ovector[2*6];
                int arg_len = ovector[2*6 + 1] - ovector[2*6];
                bytecode_pos += instr_byte_size(instr, instr_len, arg, arg_len);
        }
        return 1;
}

void parse_deinit() {
        pcre2_match_data_free(line_matchd);
        pcre2_code_free(line_re);
        pcre2_match_data_free(arg_matchd);
        pcre2_code_free(arg_re);
        pcre2_match_data_free(str_matchd);
        pcre2_code_free(str_re);
        pcre2_match_data_free(size_matchd);
        pcre2_code_free(size_re);
}
