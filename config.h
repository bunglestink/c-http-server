#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "dictionary.h"


// Route to serve from filesystem.
#define ROUTE_TYPE_FILE 0

// Route to serve from CGI.
#define ROUTE_TYPE_CGI 1

// Route to serve as HTTP proxy (unimplemented).
#define ROUTE_TYPE_HTTP 2


typedef struct Rotue {
  // Path right now is prefix only.
  char* path;
  int type;
  void* config;
} Route;


typedef struct FileConfig {
  char* local_path;
} FileConfig;


typedef struct CgiConfig {
  char* local_path;

  // Maps from file extensions to executable.
  // ex: "py" => "/usr/bin/python"
  Dictionary* file_ext_to_cmd;
} CgiConfig;


typedef struct HttpConfig {
  char* remote_host;
  int remote_port;
} HttpConfig;


typedef struct Config {
  int port;
  int routes_count;
  Route* routes;
} Config;


// TODO: Consider reading config from file instead of static configuration.
Config* get_config();
void Config_delete(Config* config);

#endif
