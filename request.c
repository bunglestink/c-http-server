#include <stdio.h>
#include <string.h>
#include "lib.h"
#include "request.h"


static char* get_method(char* raw_request, size_t* first_path_index);
static char* get_http_version(char* raw_request, size_t first_http_index, size_t* first_header_index);
static char* get_path(char* raw_request, size_t first_path_index, size_t* fist_http_index);
static Dictionary* get_headers(char* raw_request, size_t first_header_index, size_t* first_body_index);
static char* get_body(char* raw_request, size_t first_body_index);


Request* Request_new(char* raw_request) {
  size_t i;
  Request* request = (Request*) x_malloc(sizeof(Request));
  request->method = NULL;
  request->path = NULL;
  request->headers = NULL;
  request->body = NULL;

  request->method = get_method(raw_request, &i);
  if (request->method == NULL) {
    Request_delete(request);
    return NULL;
  }

  request->path = get_path(raw_request, i, &i);
  if (request->path == NULL) {
    Request_delete(request);
    return NULL;
  }

  char* http_version = get_http_version(raw_request, i, &i);
  if (http_version == NULL) {
    Request_delete(request);
    return NULL;
  }
  x_free(http_version);  // TODO: Consider using this.

  request->headers = get_headers(raw_request, i, &i);
  if (request->headers == NULL) {
    Request_delete(request);
    return NULL;
  }

  request->body = get_body(raw_request, i);
  if (request->body == NULL) {
    Request_delete(request);
    return NULL;
  }
  return request;
}


char* get_method(char* raw_request, size_t* first_path_index) {
  size_t sp = index_of(raw_request, ' ');
  size_t lf = index_of(raw_request, '\n');
  if (sp < 1 || lf < sp) {
    return NULL;
  }
  char* method = (char*) x_malloc((sp + 1) * sizeof(char));
  strncpy(method, raw_request, sp);
  method[sp] = '\0';
  // TODO: Be tolerant with whitespace.
  *first_path_index = sp + 1;
  return method;
}


char* get_path(char* raw_request, size_t first_path_index, size_t* first_http_index) {
  char* request = &raw_request[first_path_index];
  size_t sp = index_of(request, ' ');
  size_t lf = index_of(request, '\n');
  if (sp == -1 || lf < sp) {
    return NULL;
  }
  char* path = (char*) x_malloc((sp + 1) * sizeof(char));
  strncpy(path, request, sp);
  path[sp] = '\0';
  // TODO: Be tolerant with whitespace.
  *first_http_index = first_path_index + sp + 1;
  return path;
}


char* get_http_version(char* raw_request, size_t first_http_index, size_t* first_header_index) {
  char* request = &raw_request[first_http_index];
  // HTTP-version must be like "HTTP/1.1"
  size_t lf = index_of(request, '\n');
  // LF should be at index 8 or 9 (allowing missing CR).
  if (lf < 8 || lf > 9) {
    return NULL;
  }
  if (lf == 9 && request[8] != '\r') {
    return NULL;
  }
  char* http_version = (char*) x_malloc(9 * sizeof(char));
  strncpy(http_version, request, lf);
  http_version[8] = '\0';
  *first_header_index = first_http_index + lf + 1;
  return http_version;
}


Dictionary* get_headers(char* raw_request, size_t first_header_index, size_t* first_body_index) {
  char* request = &raw_request[first_header_index];
  Dictionary* headers = Dictionary_new();
  size_t lf;
  size_t processed = 0;
  while ((lf = index_of(request, '\n')) > 1) {
    if (!add_next_header(headers, request, lf)) {
      Dictionary_delete(headers);
      return NULL;
    }
    request = &request[lf + 1];
    processed += (lf + 1);
  }
  if (lf == -1) {
    Dictionary_delete(headers);
    return NULL;
  }
  if (lf == 1 && request[0] != '\r') {
    Dictionary_delete(headers);
    return NULL;
  }
  request = &request[lf + 1];
  *first_body_index = first_header_index + processed + lf + 1;
  return headers;
}


// Adds header to dict from first line of request.
int add_next_header(Dictionary* dict, char* request, size_t lf) {
  size_t colon = index_of(request, ':');
  if (colon < 1 || lf < colon) {
    return 0;
  }
  // TODO: Trim key?
  char* key = (char*) x_malloc((1 + colon) * sizeof(char));
  strncpy(key, request, colon);
  key[colon] = '\0';
  // TODO: Trim value.
  size_t size = lf - colon;
  char* value = (char*) x_malloc((1 + size) * sizeof(char));
  strncpy(value, &request[colon + 1], size);
  value[size] = '\0';

  Dictionary_set(dict, key, value);
  return 1;
}


char* get_body(char* raw_request, size_t first_body_index) {
  int len = strlen(raw_request);
  // TODO: Consider validating the body here (content-length?).
  return &raw_request[first_body_index];
}


void Request_delete(Request* request) {
  Dictionary_delete(request->headers);
  // Note: request->body is a part of raw_request.
  x_free(request->method);
  x_free(request->path);
  x_free(request);
}
