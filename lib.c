#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib.h"


/**
 * Determines whether a character is whitespace.
 */
int is_whitespace(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}


/**
 * Determines whether a character is null terminator or whitespace.
 */
int is_null_or_whitespace(char c) {
  return is_whitespace(c) || c == '\0';
}


/**
 * Gets the first index of target in input or -1 if not found.
 */
size_t index_of(char* input, char target) {
  size_t len = strlen(input);
  size_t i;
  for (i = 0; i < len; i++) {
    if (input[i] == target) {
      return i;
    }
  }
  return -1;
}


/**
 * Gets the last index of target in input or -1 if not found.
 */
size_t last_index_of(char* input, char target) {
  size_t last_index = -1;
  size_t len = strlen(input);
  size_t i;
  for (i = 0; i < len; i++) {
    if (input[i] == target) {
      last_index = i;
    }
  }
  return last_index < len ? last_index : -1;
}


/**
 * malloc proxy to handle memory erros by logging and crashing.body
 */
void* x_malloc(size_t size) {
  void* ptr = malloc(size);
  if (ptr == NULL) {
    fail("Out of memory.");
  }
  return ptr;
}


/**
 * free proxy, currently just a passthrough.
 */
void x_free(void* ptr) {
  free(ptr);
}


/**
 * Fail, logging a message and exiting.body
 */
void fail(char* error_message) {
  perror(error_message);
  exit(1);
}
