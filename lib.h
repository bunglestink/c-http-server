#ifndef _LIB_H_
#define _LIB_H_

#include "stdlib.h"


void fail(char* error_message);
int is_whitespace(char c);
int is_null_or_whitespace(char c);
size_t index_of(char* input, char target);
size_t last_index_of(char* input, char target);
void* x_malloc(size_t size);
void x_free(void* ptr);


#endif
