# Simple build script.

gcc \
  config.c \
  dictionary.c \
  lib.c \
  main.c \
  request.c \
  request_handler.c \
  -o server
