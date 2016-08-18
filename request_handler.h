#ifndef _REQUEST_HANDLER_H_
#define _REQUEST_HANDLER_H_

// TODO: Probably enlarge this.
#define REQUEST_BUFFER_SIZE 65536

void handle_request(int connection_fd, char* buffer);

#endif
