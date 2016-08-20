#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib.h"


/**
 * Determines whether a file exists.
 */
int file_exists(char* path) {
  FILE* file;
  if ((file = fopen(path, "r"))) {
    fclose(file);
    return 1;
  }
  return 0;
}


/**
 * Gets the file name from a file path.
 * Result is a substring of path, not new char array.
 */
char* get_file_name(char* path) {
  int last_separator_index = last_index_of(path, '/');
  return &path[last_separator_index + 1];
}


/**
 * Gets the extension from a file name.
 * Result is a substring of path, not new char array.
 */
char* get_extension(char* file_name) {
  int dot_index = last_index_of(file_name, '.');
  if (dot_index == -1) {
    int len = strlen(file_name);
    return &file_name[len];
  }
  return &file_name[dot_index + 1];
}


/**
 * Gets the uppercase version of a character.
 */
char to_upper(char c) {
  if (c < 'a' || c > 'z') {
    return c;
  }
  char diff = 'a' - 'A';
  return c - diff;
}


/**
 * Determines whether a character is a 0-9 numeral.
 */
int is_numeral(char c) {
  return '0' <= c && c <= '9';
}


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
