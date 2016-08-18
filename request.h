#ifndef _REQUEST_H_
#define _REQUEST_H_

#include "dictionary.h"

typedef struct RequestTag {
  char* method;
  char* path;
  Dictionary* headers;
  char* body;
} Request;

Request* Request_new(char* raw_request);
void Request_delete(Request* request);

#endif

