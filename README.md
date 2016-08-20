# Server

Runs on linux, probably anything posix right now.  Supports serving files and
CGI scripts.

## Build

    $ ./build.sh

## Run

Build, then run:

    $ ./server

## Configure

The server is currently configured to serve www/ as the root with cgi-bin/
mounted at /cgi-bin/ as well.  Serves on port 7777.  See or edit configuration
in config.c.

Eventually this will be moved to some sort of configuration file.

