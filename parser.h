#pragma once

#include <stdio.h>

#define MAXLINES 1000
#define MAXCOLS 100
#define MAXLBL 50

#define LINE_PATTERN (PCRE2_SPTR)"^[ \\t]*([^: ]+:)?[ \\t]*(\\.?[a-zA-Z]+)?[ \\t]*((.*?)(,[idnsxf]{1,3})|(.*))?"
#define ARG_PATTERN (PCRE2_SPTR)"(0x[A-Fa-f0-9]{4}|0x[A-Fa-f0-9]{2}(?=$))?([\\+-]?\\d+)?('.')?(.+)?"
#define STR_PATTERN (PCRE2_SPTR)"\"(.*)\"(?=$)"
#define PARSE_SIZE (PARSE_SIZE)"(?<=^)(\\d+)(?=$)"

void parse_deinit();
int parse_init(FILE* file);

int parse();

void print_parsed();

struct label_info {
        char *id;
        size_t id_len;
        size_t pos;
};
