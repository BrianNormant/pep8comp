#include "parser.h"

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

void
printf_bytes(char *program, uint16_t size)
{
        assert(_initialized);
        uint16_t pos = 0;
        while (pos < size) {
                printf("|0x%.2hx¦ %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx|", 
                        pos,
                        program[pos],
                        program[pos+1],
                        program[pos+2],
                        program[pos+3],
                        program[pos+4],
                        program[pos+5],
                        program[pos+6],
                        program[pos+7]);
                printf("%c%c%c%c%c%c%c%c|\n",
isgraph(program[pos])?program[pos]:'?',
isgraph(program[pos+1])?program[pos+1]:'?',
isgraph(program[pos+2])?program[pos+2]:'?',
isgraph(program[pos+3])?program[pos+3]:'?',
isgraph(program[pos+4])?program[pos+4]:'?',
isgraph(program[pos+5])?program[pos+5]:'?',
isgraph(program[pos+6])?program[pos+6]:'?',
isgraph(program[pos+7])?program[pos+7]:'?'
                        );
                pos+=8;
        }
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
parse(char *program, uint16_t *program_size)
{
        assert(_initialized);
        
        size_t *ovector;
        int rc;
        // first pass, identify labels and their positions in the final bytecode
        uint16_t bytecode_pos = 0; 
        // Shouldn't be bigger than the accessible adressing page of 16 bit
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
        printf("Parsing labels success: Bytecode size: %hu\n", bytecode_pos);

        // Second pass, convert each instruction to bytecode along with arguments
        char bytecode[bytecode_pos];
        bytecode_pos = 0;
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

                PCRE2_SPTR instr = line + ovector[2*2];
                int instr_len = ovector[2*2 + 1] - ovector[2*2];
                PCRE2_SPTR arg = line + ovector[2*6];
                int arg_len = ovector[2*6 + 1] - ovector[2*6];
                if (!instr_byte_code(
                        instr,
                        instr_len,
                        arg,
                        arg_len,
                        bytecode, &bytecode_pos)) {
                        perror("Error when generating the bytecode\n");
                        parser_failure();
                        return 0;
                }
        }
        memcpy(program, bytecode, bytecode_pos);
        *program_size  = bytecode_pos;
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
