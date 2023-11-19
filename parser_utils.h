#pragma once
#pragma GCC diagnostic ignored "-Wunused-variable"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "sugar.h"

#define MAXLINES 1000
#define MAXCOLS 100
#define MAXLBL 50

#define LINE_PATTERN (PCRE2_SPTR)"\\s*(.*:)?\\s*(\\.?\\w+)?\\s*((.*?)(,[idnsxf]{1,3})|(.*))?"
#define ARG_PATTERN (PCRE2_SPTR)"(0x[A-Fa-f0-9]{4}|0x[A-Fa-f0-9]{2}(?=$))?([\\+-]?\\d+)?('.')?(.+)?"
#define STR_PATTERN (PCRE2_SPTR)"\"(.*)\"(?=$)"
#define PARSE_SIZE (PARSE_SIZE)"(?<=^)(\\d+)(?=$)"

struct label_info {
        char *id;
        size_t id_len;
        uint16_t pos;
};

extern int _initialized;
extern char source_lines[MAXLINES][MAXCOLS];
extern int lines_cnt;
extern struct label_info source_labels[MAXLBL];
extern int label_cnt;
extern pcre2_code *line_re;
extern pcre2_match_data *line_matchd;
extern pcre2_code *arg_re;
extern pcre2_match_data *arg_matchd;
extern pcre2_code *str_re;
extern pcre2_match_data *str_matchd;
extern pcre2_code *size_re;
extern pcre2_match_data *size_matchd;
extern int errornumber;
extern unsigned long erroroffset;

extern int
match_line(PCRE2_SPTR subject, int line, int *rc, size_t **ov);

extern int
match_arg(PCRE2_SPTR subject, int line, int *rc, size_t **ov);

extern int
match_str(PCRE2_SPTR subject, size_t len, int line, int *rc, size_t **ov);

extern int
match_size(PCRE2_SPTR subject, int line, int *rc, size_t **ov);

extern int
label_exist(PCRE2_SPTR id, int id_len);

extern int
get_label(PCRE2_SPTR id, int id_len);

extern int
add_label(PCRE2_SPTR id, size_t len, size_t pos);

extern int
instr_is_instr(PCRE2_SPTR id, int id_len);

extern int
instr_byte_code(PCRE2_SPTR instr, size_t instr_len,
                PCRE2_SPTR arg, size_t arg_len,
                char *code, uint16_t *code_pos);

extern size_t 
instr_byte_size(PCRE2_SPTR instr, size_t instr_len,
                PCRE2_SPTR arg, size_t arg_len); 

#define INSTR_CNT (sizeof(INSTR_LIST)/sizeof(INSTR_LIST[0]))
static const char *INSTR_LIST[] = {
        "stop",
        "movspa",
        "movflga",
        "br",
        "brle",
        "brlt",
        "breq",
        "brne",
        "brge",
        "brgt",
        "brv",
        "brc",
        "call",
        "nota",
        "notx",
        "nega",
        "negx",
        "asla",
        "aslx",
        "asra",
        "asrx",
        "rola",
        "rolx",
        "rora",
        "rorx",
        "nopn",
        "nop",
        "deci",
        "deco",
        "stro",
        "chari",
        "charo",
        "ret0",
        "addsp",
        "subsp",
        "adda",
        "addx",
        "suba",
        "subx",
        "anda",
        "andx",
        "ora",
        "orx",
        "cpa",
        "cpx",
        "lda",
        "ldx",
        "ldbytea",
        "ldbytex",
        "sta",
        "stx",
        "stbytea",
        "stbytex",
};

static const char EXP_SEQUENCES[] = {
        'n',
        'r',
        'x',
        't',
};
