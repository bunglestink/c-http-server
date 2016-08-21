#ifndef _REQUEST_H_
#define _REQUEST_H_

#include <netinet/in.h>
#include "dictionary.h"

typedef struct RequestTag {
  char* method;
  char* path;
  Dictionary* headers;
  char* body;
  int content_length;
  int remote_port;
  char* remote_ip_address;
} Request;

Request* Request_new(char* raw_request, int raw_request_length, struct sockaddr_in* client_addr);
void Request_delete(Request* request);

#endif

