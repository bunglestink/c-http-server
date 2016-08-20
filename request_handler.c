#include <stdio.h>
#include <string.h>
#include "config.h"
#include "lib.h"
#include "request.h"
#include "request_handler.h"

static int file_exists(char* file_path);
char* get_cgi_command(Request* request, Route* route);
char* get_cmd_with_env(char* cmd, Dictionary* env);
static char* get_extension(char* file_name);
static char* get_file_name(char* path);
static long get_file_size(FILE* file);
char* get_header_key(char* header);
static char* get_local_path(Request* request, Route* route);
void handle_cgi_request(int connfd, Request* request, Route* route);
void handle_file_request(int connfd, Request* request, Route* route);
int is_numeral(char c);
char to_upper(char c);
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
  char buffer[RESPONSE_BUFFER_SIZE];

  char* cmd = get_cgi_command(request, route);
  if (cmd == NULL) {
    printf("File not found: %s\n", request->path);
    write_response(connfd, "HTTP/1.0 404 Not Found\r\nContent-Length: 15\r\n\r\nFile Not Found\n");
    return;
  }
  FILE* proc = popen(cmd, "r");
  if (!proc) {
    printf("Error running command: %s\n", cmd);
    write_response(connfd, "HTTP/1.0 500 Internal Server Error\r\nContent-Length: 22\r\n\r\nInternal Server Error\n");
    x_free(cmd);
    return;
  }

  write_response(connfd, "HTTP/1.0 ");
  int status_written = 0;
  int bytes_read;
  while ((bytes_read = fread(buffer, 1, RESPONSE_BUFFER_SIZE, proc)) != 0) {
    if (bytes_read < 0) {
      perror("ERROR: Reading file.");
      break;
    }
    if (!status_written) {
      status_written = 1;
      if (bytes_read < 4 ||
          !is_numeral(buffer[0]) ||
          !is_numeral(buffer[1]) ||
          !is_numeral(buffer[2]) ||
          (buffer[3] != ' ' && buffer[3] != '\t')) {

        write_response(connfd, "200 OK\r\n");
      }
    }
    if (!write_response_bytes(connfd, buffer, bytes_read)) {
      break;
    }
  }

  x_free(cmd);
  pclose(proc);
}


char* get_cgi_command(Request* request, Route* route) {
  char* local_path = get_local_path(request, route);
  if (!file_exists(local_path)) {
    x_free(local_path);
    return NULL;
  }
  CgiConfig* config = (CgiConfig*) route->config;
  // TODO: Support path after script.
  char* extension = get_extension(local_path);
  char* cmd = Dictionary_get(config->file_ext_to_cmd, extension);
  char* cmd_to_execute;
  if (cmd == NULL) {
    cmd_to_execute = local_path;
  } else {
    int size = strlen(cmd) + 1 + strlen(local_path) + 1;
    cmd_to_execute = (char*) x_malloc(size);
    sprintf(cmd_to_execute, "%s %s", cmd, local_path);
    cmd_to_execute[size] = '\0';
  }

  Dictionary* env = Dictionary_new();
  // Set standard CGI environment variables.
  Dictionary_set(env, "AUTH_TYPE", "");
  Dictionary_set(env, "CONTENT_LENGTH", "");
  Dictionary_set(env, "CONTENT_TYPE", "");
  Dictionary_set(env, "GATEWAY_INTERFACE", "");
  Dictionary_set(env, "PATH_INFO", request->path);
  // TODO: Support for path after script here.
  Dictionary_set(env, "PATH_TRANSLATED", "");
  Dictionary_set(env, "REDIRECT_STATUS", "200");  // Required for php-cgi
  Dictionary_set(env, "REMOTE_ADDR", "");
  Dictionary_set(env, "REMOTE_HOST", "");
  Dictionary_set(env, "REMOTE_IDENT", "");
  Dictionary_set(env, "REMOTE_USER", "");
  Dictionary_set(env, "REQUEST_METHOD", request->method);
  Dictionary_set(env, "SCRIPT_FILENAME", local_path);  // Required for php-cgi
  Dictionary_set(env, "SCRIPT_NAME", local_path);
  Dictionary_set(env, "SERVER_NAME", "");
  Dictionary_set(env, "SERVER_PORT", "");
  Dictionary_set(env, "SERVER_PROTOCOL", "");
  Dictionary_set(env, "SERVER_SOFTWARE", "");

  DictionaryEntry* entries = Dictionary_get_entries(request->headers);
  int i;
  for (i = 0; i < request->headers->size; i++) {
    char* header_key = get_header_key(entries[i].key);
    Dictionary_set(env, header_key, entries[i].value);
  }
  x_free(entries);

  char* cmd_with_env = get_cmd_with_env(cmd_to_execute, env);
  x_free(local_path);
  x_free(cmd_to_execute);
  Dictionary_delete(env);
  return cmd_with_env;
}


char* get_header_key(char* header) {
  size_t len = strlen(header);
  size_t key_len = 6 + len;
  char* key = (char*) x_malloc(key_len * sizeof(char));
  sprintf(key, "HTTP_%s", header);
  key[key_len - 1] = '\0';

  int i;
  for (i = 0; i < key_len; i++) {
    key[i] = to_upper(key[i]);
    if (key[i] == '-') {
      key[i] = '_';
    }
  }
  return key;
}


char* get_cmd_with_env(char* cmd, Dictionary* env) {
  size_t total_length = 0;
  DictionaryEntry* entries = Dictionary_get_entries(env);
  int i;
  for (i = 0; i < env->size; i++) {
    DictionaryEntry* entry = &entries[i];
    // Format: 'KEY="VALUE" '
    total_length += strlen(entry->key) + 2 + strlen(entry->value) + 2;
  }
  total_length += strlen(cmd);
  total_length += 1;  // null terminator.

  printf("TL: %i\n", (int)total_length);
  char* cmd_with_env = (char*) x_malloc(total_length * sizeof(char));
  memset(cmd_with_env, 0, total_length);
  char* cmd_ptr = cmd_with_env;
  for (i = 0; i < env->size; i++) {
    DictionaryEntry* entry = &entries[i];
    sprintf(cmd_ptr, "%s=\"%s\" ", entry->key, (char*) entry->value);
    int len = strlen(entry->key) + 2 + strlen(entry->value) + 2;
    cmd_ptr = &cmd_ptr[len];
  }
  sprintf(cmd_ptr, "%s", cmd);
  printf("CMD: %s\n", cmd_with_env);
  return cmd_with_env;
}


char to_upper(char c) {
  if (c < 'a' || c > 'z') {
    return c;
  }
  char diff = 'a' - 'A';
  return c - diff;
}

int is_numeral(char c) {
  return '0' <= c && c <= '9';
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
