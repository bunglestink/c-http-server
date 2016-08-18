#include <stdio.h>
#include <string.h>
#include "lib.h"
#include "request.h"
#include "request_handler.h"

static char* get_extension(char* file_name);
static char* get_file_name(char* path);
static char* get_local_path(char* path);
static int file_exists(char* file_path);
static void write_file(int connfd, char* path);
static int write_response(int connfd, char* raw_response);
static int write_response_bytes(int connfd, char* raw_response, size_t num_bytes);
static void write_type_headers(int connfd, char* file_name);

// Response buffer size of 0.25M
#define RESPONSE_BUFFER_SIZE 262144


void handle_request(int connfd, char* buffer) {
  memset(buffer, 0, REQUEST_BUFFER_SIZE);
  if (read(connfd, buffer, REQUEST_BUFFER_SIZE) < 0) {
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


char* get_file_name(char* path) {
  int last_separator_index = last_index_of(path, '/');
  return &path[last_separator_index + 1];
}


char* get_extension(char* file_name) {
  int dot_index = last_index_of(file_name, '.');
  if (dot_index == -1) {
    int len = strlen(file_name);
    return &file_name[len];
  }
  return &file_name[dot_index + 1];
}


void write_file(int connfd, char* path) {
  FILE* file;
  char* local_path = get_local_path(path);
  if (!(file = fopen(local_path, "rb"))) {
    printf("Unable to open file: %s\n", local_path);
    write_response(connfd, "HTTP/1.0 500 Internal Server Error\n\n");
    return;
  }
  char* file_name = get_file_name(path);
  write_response(connfd, "HTTP/1.0 200 OK\n");
  write_type_headers(connfd, file_name);
  write_response(connfd, "\n");

  int bytes_read;
  char* buffer = (char*) x_malloc(RESPONSE_BUFFER_SIZE * sizeof(char));
  while ((bytes_read = fread(buffer, 1, RESPONSE_BUFFER_SIZE, file)) != 0) {
    if (bytes_read < 0) {
      perror("ERROR: Reading file.");
      break;
    }
    if (!write_response_bytes(connfd, buffer, bytes_read)) {
      break;
    }
  }
  printf("\nbytes: %i\n", (int)XXtotal);
  fclose(file);
}


int write_response(int connfd, char* raw_response) {
  size_t len = strlen(raw_response);
  return write_response_bytes(connfd, raw_response, len);
}


int write_response_bytes(int connfd, char* response, size_t num_bytes) {
  char* ptr = response;
  int bytes_to_write = num_bytes;
  while (bytes_to_write > 0) {
    int bytes_written = write(connfd, ptr, bytes_to_write);
    if (bytes_written < 0) {
      perror("ERROR: Unable to send response.");
      return 0;
    }
    bytes_to_write -= bytes_written;
    ptr += bytes_written;
  }
  return 1;
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
