#include "parser_utils.h"


int _initialized = 0;
char source_lines[MAXLINES][MAXCOLS];
int lines_cnt = 0;
struct label_info source_labels[MAXLBL];
int label_cnt = 0;
pcre2_code *line_re;
pcre2_match_data *line_matchd;
pcre2_code *arg_re;
pcre2_match_data *arg_matchd;
pcre2_code *str_re;
pcre2_match_data *str_matchd;
pcre2_code *size_re;
pcre2_match_data *size_matchd;
int errornumber;
unsigned long erroroffset;

// Matching
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

// Labels

int // Check if label is already defined
label_exists(PCRE2_SPTR id, int id_len)
{
        FORI(i, 0, label_cnt) {
                if (strncmp(source_labels[i].id, (char*)id, id_len) == 0) return 1;
        }
        return 0;
}

int // return index of label id in source_labels, if not found, return -1
get_label(PCRE2_SPTR id, int id_len)
{
        FORI(i, 0, label_cnt) {
                if (strncmp(source_labels[i].id, (char*)id, id_len) == 0) return i;
        }
        return -1;
}

int
add_label(PCRE2_SPTR id, size_t len, size_t pos) {
        if (label_cnt >= MAXLBL) {
                perror("Too many labels");
                return 0;
        }
        if (label_exists(id, len)) {
                fprintf(stderr, "Label [%.*s] already defined\n", (int)len, id);
                return 0;
        }
        source_labels[label_cnt].id = (char*)id;
        source_labels[label_cnt].id_len = len;
        source_labels[label_cnt].pos = pos;
        printf("Found Label \"%.*s\" at position %hu\n",
                        (int)source_labels[label_cnt].id_len,
                        source_labels[label_cnt].id,
                        source_labels[label_cnt].pos);
        label_cnt++;
        return 1;
}

// Bytecode generation
int instr_is_instr(PCRE2_SPTR id, int id_len) {
        FORI(i, 0, (int)INSTR_CNT)
                if (strncmp(INSTR_LIST[i], (char*)id, id_len) == 0)
                        return 1;
        return 0;
}

int
instr_byte_code(PCRE2_SPTR instr, size_t instr_len,
                PCRE2_SPTR arg, size_t arg_len,
                char *code, uint16_t *code_pos)
{
        if (instr_len == 0) return 1; // No instruction

        char instr_r[instr_len+1];
        memcpy(instr_r, (char*)instr, instr_len);
        instr_r[instr_len] = '\0';

        FORI(i, 0, (int)instr_len) instr_r[i] = tolower(instr_r[i]);
        
        printf("   \x1b[31m Instruction: %.*s %.*s\n \x1b[0m", (int)instr_len, instr_r, (int)arg_len, arg);


        if (strncmp((char*)instr_r, ".byte", instr_len) == 0) {
                char arg_r[7];
                int d; unsigned char c;
                uint16_t ud;
                if (arg_len > 6) {
                        fprintf(stderr, "Wrong argument for .BYTE [%.*s]\n", (int)instr_len, instr_r);
                        return 0;
                }
                sprintf(arg_r, "%.*s", (int)arg_len, (char*)arg);// First 6 char, biggest argument: '\xFF'\0
                
                // Try to parse as hex 0xFF
                if (sscanf((char*)arg_r, "%hhx", &c) == 1) {
                        code[*code_pos] = (signed char)d;
                } 
                // Try to parse as decimal -127;255
                if (sscanf((char*)arg_r, "%d", &d) == 1) {
                        if (d < -127 || d > 255) {
                                fprintf(stderr, "Wrong argument for byte[%s]", arg_r);
                                return 1;
                        }
                        code[*code_pos] = (signed char)d;
                } 
                // Try to parse as char 'a'
                if (sscanf((char*)arg_r, "'%c'", &c) == 1) {
                        code[*code_pos] = c;
                } else
                // Try to parse as char '\n'
                if (sscanf((char*)arg_r, "'\\%c'", &c) == 1) {
                        switch (c) {
                        case 'n': // fallthrough
                        case 'r': code[*code_pos] = '\n';
                                  break;
                        case 't': code[*code_pos] = '\t';
                                  break;
                        case 'x': perror("\\x Must be followed by 2 hexditi\n");
                                  return 0;
                        default: fprintf(stderr, "[\\%c] is not implemented yet", c);
                                 return 0;
                        }
                } else
                // Try to parse as char '\x0F'
                if (sscanf((char*)arg_r, "'\\x%hx'", &ud) == 1) {
                        code[*code_pos] = (signed char)d;
                } else {
                        fprintf(stderr, "Cannot parse argument [%.*s] [%s]\n", (int)instr_len, instr_r, arg_r);
                }
                *code_pos += 1;
                return 1;
        } else if (strncmp((char*)instr_r, ".word", instr_len) == 0) {
                // Try to parse as hex 0x08AF or decimal -32767 >= 65535
                int16_t d;

                if (sscanf((char*)arg, "%hd", &d) == 1) {
                        code[*code_pos] = d;
                        *code_pos += 2;
                        return 1;
                }
                if (sscanf((char*)arg, "%hx", (uint16_t*)&d) == 1) {
                        code[*code_pos] = d;
                        *code_pos += 2;
                        return 1;
                }
                perror("Wrong argument for .word");
                return 0;
        } else if (strncmp((char*)instr_r, ".block", instr_len) == 0) {
                // Try to parse arg as positive int16
                uint16_t d;
                if (sscanf((char*)arg, "%hu", &d) == 1) {
                        FORI(i, 0, (int)d) {
                                code[*code_pos] = (char)0;
                                *code_pos += 1;
                        }
                        return 1;
                }
                perror("Wrong argument for .block");
                return 0;
        } else if (strncmp((char*)instr_r, ".addrss", instr_len) == 0) {
                // get the address of label
                int lbl_i = get_label(arg, arg_len);
                if (lbl_i == -1) {
                        fprintf(stderr, "Reference to undefined label [%.*s]\n", (int)arg_len, arg);
                        return 0;
                }
                code[*code_pos] = source_labels[lbl_i].pos;
                *code_pos += 2;
                return 1;
        } else if (strncmp((char*)instr_r, ".ascii", instr_len) == 0) {
                // TODO, don't automagically add null terminator byte
                FORI(i, 0, (int)arg_len) {
                        code[*code_pos] = arg[i];
                        *code_pos += 1;
                }
                code[*code_pos] = '\0';
                *code_pos += 1;
                return 1;
        } else if (instr_is_instr((PCRE2_SPTR)instr_r, instr_len)) {
                // TODO bytecode for each instructions
                return 1;
        }
        perror("Unknown instruction");
        return 0;
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
        if (instr_len == 0) return 0;
        char instr_r[instr_len+1];
        printf("  preinstr: [%.*s]\n", (int)instr_len, instr);
        memcpy(instr_r, (char*)instr, instr_len);
        instr_r[instr_len] = '\0';
        
        FORI(i, 0, (int)instr_len) instr_r[i] = (char)tolower(instr_r[i]);
        printf("  arg: [%.*s]\n", (int)arg_len, arg);
        printf("  instr: [%.*s]\n", (int)instr_len, instr_r);


        if (instr_r[0] == '.') {
                if (strncmp((char*)instr_r, ".byte", instr_len) == 0)
                        return 1;
                else if (strncmp((char*)instr_r, ".word", instr_len) == 0)
                        return 2;
                else if (strncmp((char*)instr_r, ".addrss", instr_len) == 0)
                        return 2;
                else if (strncmp((char*)instr_r, ".ascii", instr_len) == 0) {
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
                } else if (strncmp((char*)instr_r, ".block", instr_len) == 0) {
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
