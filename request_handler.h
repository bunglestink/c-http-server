#ifndef _REQUEST_HANDLER_H_
#define _REQUEST_HANDLER_H_

#include <netinet/in.h>

void handle_request(int connection_fd, struct sockaddr_in* client_addr, Config* config);

#endif
