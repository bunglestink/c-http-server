#ifndef _REQUEST_H_
#define _REQUEST_H_


typedef struct RequestTag {
  char* method;
  char* path;
  // TODO: More?
  
} Request;

Request* Request_new(char* raw_request);
void Request_delete(Request* request);

#endif

