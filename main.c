#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "request.h"

#define BUFFER_SIZE 4096


void fail(char* error_message);
int file_exists(char* file_path);
void write_response(int connfd, char* raw_response);
void write_file(int connfd, char* path);
char* get_local_path(char* path);
char* get_file_name(char* path);
char* get_extension(char* file_name);
int get_last_index_of(char* input, char target);
void write_type_headers(int connfd, char* file_name);


void main(void) {
  int listenfd = 0, connfd = 0, client_length;
  int port = 7777;
  char buffer[BUFFER_SIZE];
  struct sockaddr_in server_addr, client_addr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fail("Error: Unable to open socket.");
  }
  bzero(&server_addr, sizeof(server_addr));
  
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
    bzero(buffer, BUFFER_SIZE);
    if (read(connfd, buffer, BUFFER_SIZE) < 0) {
      fail("ERROR: Unable to read socket.");
    }

    Request* request = Request_new(buffer);
    if (request == NULL) {
      printf("Bad request received: %s\n", buffer);
      write_response(connfd, "HTTP/1.0 400 Bad Request\nContent-Length: 0\n\n");
    } else if (strcmp(request->method, "GET") != 0) {
      printf("Method not allowed: %s\n", request->method);
      write_response(connfd, "HTTP/1.0 405 Method Not Allowed\nContent-Length: 0\n\n");
    } else if (!file_exists(request->path)) {
      printf("File not found: %s\n", request->path);
      write_response(connfd, "HTTP/1.0 404 Not Found\nContent-Length: 0\n\nFile Not Found\n");
    } else {
      printf("%s %s\n", request->method, request->path);
      write_file(connfd, request->path);
      Request_delete(request);
    }

    close(connfd);
  }
}


char* get_local_path(char* path) {
  // TODO: Make this configurable and safe.
  return &path[1];
}


int file_exists(char* path) {
  FILE* file;
  char* local_path = get_local_path(path);
  if ((file = fopen(local_path, "r"))) {
    fclose(file);
    return 1;
  }
  return 0;
}


void write_file(int connfd, char* path) {
  FILE* file;
  char* local_path = get_local_path(path);
  if (!(file = fopen(local_path, "r"))) {
    printf("Unable to open file: %s\n", local_path);
    write_response(connfd, "HTTP/1.0 500 Internal Server Error\n\n");
    return;
  }
  char* file_name = get_file_name(path);
  write_response(connfd, "HTTP/1.0 200 OK\n");
  write_type_headers(connfd, file_name);
  write_response(connfd, "\n");
  // TODO: Replace with buffered read/write.
  int c;
  while ((c = fgetc(file)) != EOF) {
    write_response(connfd, (char*) &c);
  }
  fclose(file);
}


void write_type_headers(int connfd, char* file_name) {
  char* extension = get_extension(file_name);
  // TODO: Mime-type fun :)
  if (strcmp(extension, "html") == 0) {
    write_response(connfd, "Content-Type: text/html\n");
    return;
  }
  if (strcmp(extension, "css") == 0) {
    write_response(connfd, "Content-Type: text/css\n");
    return;
  }
  if (strcmp(extension, "js") == 0) {
    write_response(connfd, "Content-Type: text/javascript\n");
    return;
  }
  write_response(connfd, "Content-Disposition: attachment; filename=");
  write_response(connfd, file_name);
  write_response(connfd, "\n");
}


char* get_file_name(char* path) {
  int last_separator_index = get_last_index_of(path, '/');
  return &path[last_separator_index + 1];
}


char* get_extension(char* file_name) {
  int dot_index = get_last_index_of(file_name, '.');
  if (dot_index == -1) {
    int len = strlen(file_name);
    return &file_name[len];
  }
  return &file_name[dot_index + 1];
}


int get_last_index_of(char* input, char target) {
  int last_index = -1;
  int len = strlen(input);
  int i;
  for (i = 0; i < len; i++) {
    if (input[i] == target) {
      last_index = i;
    }
  }
  return last_index;
}


void write_response(int connfd, char* raw_response) {
  int len = strlen(raw_response);
  if (write(connfd, raw_response, len) < 0) {
    perror("ERROR: Unable to send response.");
  }
}


void fail(char* error_message) {
  perror(error_message);
  exit(1);
}

