#include <stdio.h>
#include <string.h>
#include "config.h"
#include "lib.h"
#include "request.h"
#include "request_handler.h"

static char* get_extension(char* file_name);
static char* get_file_name(char* path);
static long get_file_size(FILE* file);
static char* get_local_path(Request* request, Route* route);
static int file_exists(char* file_path);
void handle_cgi_request(int connfd, Request* request, Route* route);
void handle_file_request(int connfd, Request* request, Route* route);
static void write_content_length_header(int connfd, long size);
static void write_file(int connfd, char* path);
static int write_response(int connfd, char* raw_response);
static int write_response_bytes(int connfd, char* raw_response, size_t num_bytes);
static void write_type_headers(int connfd, char* file_name);

// Response buffer size of 1MB
#define RESPONSE_BUFFER_SIZE 1048576


void handle_request(int connfd, char* buffer) {
  memset(buffer, 0, REQUEST_BUFFER_SIZE);
  if (read(connfd, buffer, REQUEST_BUFFER_SIZE) < 0) {
    fail("ERROR: Unable to read socket.");
  }

  Request* request = Request_new(buffer);
  if (request == NULL) {
    printf("Bad request received: %s\n", buffer);
    write_response(connfd, "HTTP/1.0 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
    return;
  }

  Config* config = get_config();
  int i;
  for (i = 0; i < config->routes_count; i++) {
    Route* route = &config->routes[i];
    if (!is_route_match(route, request)) {
      continue;
    }
    switch (route->type) {
      case ROUTE_TYPE_CGI:
        handle_cgi_request(connfd, request, route);
        break;
      case ROUTE_TYPE_FILE:
        handle_file_request(connfd, request, route);
        break;
      default:
        perror("INTERNAL ERROR: Bad config route type.");
        write_response(connfd, "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n");
    }
    break;
  }
  Config_delete(config);
  Request_delete(request);
}


int is_route_match(Route* route, Request* request) {
  // Right now we only support prefix matching.
  size_t len = strlen(route->path);
  return strncmp(request->path, route->path, len) == 0;
}


void handle_cgi_request(int connfd, Request* request, Route* route) {
  write_response(connfd, "HTTP/1.0 200 OK\r\nContent-Length: 2\r\n\r\nOK");
}


void handle_file_request(int connfd, Request* request, Route* route) {
  if (strcmp(request->method, "GET") != 0) {
    printf("Method not allowed: %s\n", request->method);
    write_response(connfd, "HTTP/1.0 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n");
    return;
  }
  char* local_path = get_local_path(request, route);
  if (!file_exists(local_path)) {
    printf("File not found: %s\n", request->path);
    write_response(connfd, "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\nFile Not Found\n");
    x_free(local_path);
    return;
  }
  printf("%s %s\n", request->method, request->path);
  write_file(connfd, local_path);
  x_free(local_path);
}


char* get_local_path(Request* request, Route* route) {
  FileConfig* file_config = (FileConfig*) route->config;

  int length = strlen(request->path) + strlen(file_config->local_path) - strlen(route->path);
  int route_path_length = strlen(route->path);
  char* file_path = &request->path[route_path_length];
  char* local_path = (char*) x_malloc(sizeof(1 + length));
  sprintf(local_path, "%s%s", file_config->local_path, file_path);
  local_path[length] = '\0';
  return local_path;
}


int file_exists(char* path) {
  FILE* file;
  if ((file = fopen(path, "r"))) {
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
  if (!(file = fopen(path, "rb"))) {
    printf("Unable to open file: %s\n", path);
    write_response(connfd, "HTTP/1.0 500 Internal Server Error\r\n\r\n");
    return;
  }
  long file_size = get_file_size(file);
  if (file_size < 0) {
    perror("ERROR: Unable to get file size.");
    write_response(connfd, "HTTP/1.0 500 Internal Server Error\r\n\r\n");
    return;
  }
  char* file_name = get_file_name(path);
  write_response(connfd, "HTTP/1.0 200 OK\r\n");
  write_type_headers(connfd, file_name);
  write_content_length_header(connfd, file_size);
  write_response(connfd, "\r\n");

  int bytes_read;
  int total = 0;
  char* buffer = (char*) x_malloc(RESPONSE_BUFFER_SIZE * sizeof(char));
  while ((bytes_read = fread(buffer, 1, RESPONSE_BUFFER_SIZE, file)) != 0) {
    total += bytes_read;
    if (bytes_read < 0) {
      perror("ERROR: Reading file.");
      break;
    }
    if (!write_response_bytes(connfd, buffer, bytes_read)) {
      break;
    }
  }
  printf("Total: %i\n", total);
  fclose(file);
  // TODO: Figure out how to ensure write is complete before exiting.
}


// Assumes at beginning of file, then seeks back.
long get_file_size(FILE* file) {
  long size;
  if (fseek(file, 0L, SEEK_END) != 0) {
    return -1;
  }
  size = ftell(file);
  if (fseek(file, 0L, SEEK_SET) != 0) {
    return -1;
  }
  return size;
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


void write_content_length_header(int connfd, long size) {
  char header[1024];
  memset(header, 0, 1024);
  sprintf(header, "Content-Length: %ld\r\n", size);
  write_response(connfd, header);
}


void write_type_headers(int connfd, char* file_name) {
  char* extension = get_extension(file_name);
  // TODO: Mime-type fun :)
  if (strcmp(extension, "html") == 0) {
    write_response(connfd, "Content-Type: text/html\r\n");
    return;
  }
  if (strcmp(extension, "css") == 0) {
    write_response(connfd, "Content-Type: text/css\r\n");
    return;
  }
  if (strcmp(extension, "js") == 0) {
    write_response(connfd, "Content-Type: text/javascript\r\n");
    return;
  }
  if (strcmp(extension, "jpg") == 0) {
    write_response(connfd, "Content-Type: image/jpeg\r\n");
    return;
  }
  if (strcmp(extension, "png") == 0) {
    write_response(connfd, "Content-Type: image/png\r\n");
    return;
  }
}
