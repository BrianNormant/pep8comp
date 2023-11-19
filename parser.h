#pragma once

#include <bits/types/sigevent_t.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "parser_utils.h"


void parse_deinit();
int parse_init(FILE* file);

int parse(char *program, uint16_t *program_size);

void print_parsed();
void printf_bytes(char *program, uint16_t size);
