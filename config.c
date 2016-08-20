#include "config.h"
#include "dictionary.h"
#include "lib.h"

static void CgiConfig_delete(CgiConfig* config);
static void FileConfig_delete(FileConfig* config);
static void HttpConfig_delete(HttpConfig* config);


Config* get_config() {
  Config* config = (Config*) x_malloc(sizeof(Config));
  config->routes_count = 2;
  config->routes = (Route*) x_malloc(2 * sizeof(Route));

  Route* cgi_route = &config->routes[0];
  cgi_route->path = "/cgi-bin/";
  cgi_route->type = ROUTE_TYPE_CGI;
  cgi_route->config = x_malloc(sizeof(CgiConfig));
  CgiConfig* cgi_config = (CgiConfig*) cgi_route->config;
  cgi_config->local_path = "cgi-bin/";
  cgi_config->file_ext_to_cmd = Dictionary_new();
  Dictionary_set(cgi_config->file_ext_to_cmd, "sh", "bash");
  Dictionary_set(cgi_config->file_ext_to_cmd, "php", "php-cgi");
  Dictionary_set(cgi_config->file_ext_to_cmd, "pl", "perl");
  Dictionary_set(cgi_config->file_ext_to_cmd, "py", "python");

  Route* www_route = &config->routes[1];
  www_route->path = "/";
  www_route->type = ROUTE_TYPE_FILE;
  www_route->config = x_malloc(sizeof(FileConfig));
  FileConfig* www_config = (FileConfig*) www_route->config;
  www_config->local_path = "www/";

  return config;
}


void Config_delete(Config* config) {
  int i;
  for (i = 0; i < config->routes_count; i++) {
    Route* route = &config->routes[i];
    switch (route->type) {
      case ROUTE_TYPE_CGI:
        CgiConfig_delete((CgiConfig*) route->config);
        break;
      case ROUTE_TYPE_FILE:
        FileConfig_delete((FileConfig*) route->config);
        break;
      case ROUTE_TYPE_HTTP:
        HttpConfig_delete((HttpConfig*) route->config);
        break;
    }
  }
  x_free(config->routes);
  x_free(config);
}


void CgiConfig_delete(CgiConfig* config) {
  Dictionary_delete(config->file_ext_to_cmd);
  x_free(config);
}

void FileConfig_delete(FileConfig* config) {
  x_free(config);
}

void HttpConfig_delete(HttpConfig* config) {
  x_free(config);
}
