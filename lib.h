#ifndef _LIB_H_
#define _LIB_H_

#include "stdio.h"
#include "stdlib.h"


void fail(char* error_message);
char* get_extension(char* file_name);
char* get_file_name(char* path);
long get_file_size(FILE* file);
int is_numeral(char c);
int is_whitespace(char c);
int is_null_or_whitespace(char c);
size_t index_of(char* input, char target);
size_t last_index_of(char* input, char target);
size_t char_count(char* input, char target);
size_t nchar_count(char* input, char target, size_t length);
char to_upper(char c);
void* x_malloc(size_t size);
void x_free(void* ptr);


#endif
