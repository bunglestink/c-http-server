#include <stdlib.h>
#include <string.h>
#include "request.h"


static char* get_method(char* raw_request);
static char* get_path(char* raw_request);
static int get_method_length(char* raw_request);


Request* Request_new(char* raw_request) {
  if (strlen(raw_request) < 4) {
    return NULL;
  }
  char* method = get_method(raw_request);
  if (method == NULL) {
    return NULL;
  }
  char* path = get_path(raw_request);
  if (path == NULL) {
    return NULL;
  }
  Request* request = (Request*) malloc(sizeof(Request));
  request->method = method;
  request->path = path;
  return request;
}


char* get_method(char* raw_request) {
  int method_len = get_method_length(raw_request);
  char* method = (char*) malloc((method_len + 1) * sizeof(char));
  strncpy(method, raw_request, method_len);
  method[method_len] = '\0';
  return method;
}


int get_method_length(char* raw_request) {
  int len = strlen(raw_request);
  int method_len = 0;
  int i;
  for (i = 0; i < len; i++) {
    if (raw_request[i] == ' ') {
      break;
    }
    // Only allow uppercase methods.
    if (raw_request[i] < 'A' || raw_request[i] > 'Z') {
      return -1;
    }
    method_len += 1;
  }
  return method_len;
}


char* get_path(char* raw_request) {
  char path_buffer[4096];
  int path_start = -1;
  int path_len = 0;
  int len = strlen(raw_request);
  int i;
  for (i = 0; i < len; i++) {
    if (raw_request[i] == ' ') {
      path_start = i + 1;
      break;
    }
  }
  if (path_start == -1) {
    return NULL;
  }
  for (path_len = 0; (path_len + path_start) < len; path_len++) {
    // TODO: Valid chars only?
    if (raw_request[path_start + path_len] == ' ') {
      break;
    }
  }
  if (path_len == 0 || (path_start + path_len) > len) {
    return NULL;
  }
  char* path = (char*) malloc((path_len + 1) * sizeof(char));
  strncpy(path, &raw_request[path_start], path_len);
  return path;
}


void Request_delete(Request* request) {
  free(request->method);
  free(request->path);
  free(request);
}

