#include <string.h>
#include <netinet/in.h>
#include "lib.h"
#include "request_handler.h"


void main(void) {
  int listenfd = 0, connfd = 0, client_length;
  int port = 7777;
  char buffer[REQUEST_BUFFER_SIZE];
  struct sockaddr_in server_addr, client_addr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fail("Error: Unable to open socket.");
  }
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    fail("ERROR: Unable to bind.");
  }
  listen(listenfd, 10);
  client_length = sizeof(client_addr);

  while (1) {
    connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_length);
    if (connfd < 0) {
      fail("ERROR: Unable to accept.");
    }
    if (0 == fork()) {
      break;
    }
    close(connfd);
  }

  close(listenfd);
  handle_request(connfd, buffer);
  close(connfd);
}
