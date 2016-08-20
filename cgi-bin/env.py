import json
import os

KEYS = [
  'AUTH_TYPE',
  'CONTENT_LENGTH',
  'CONTENT_TYPE',
  'GATEWAY_INTERFACE',
  'HTTP_ACCEPT',
  'PATH_INFO',
  'PATH_TRANSLATED',
  'REMOTE_ADDR',
  'REMOTE_HOST',
  'REMOTE_IDENT',
  'REMOTE_USER',
  'REQUEST_METHOD',
  'SCRIPT_NAME',
  'SERVER_NAME',
  'SERVER_PORT',
  'SERVER_PROTOCOL',
  'SERVER_SOFTWARE'
]


def main():
  body = '\n'.join(['%s: %s' % (k, os.environ[k]) for k in KEYS])

  print('Content-Length: %s' % len(body))
  print('Content-Type: text/plain')
  print('')
  print(body)


if __name__ == '__main__':
  main()

