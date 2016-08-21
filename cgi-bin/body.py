import os
import sys


def main():
  content_length = int(os.environ['CONTENT_LENGTH'])
  body = sys.stdin.read(content_length)
  
  response = '<h1>Request Body</h1><hr>%s' % body
  print 'Content-Type: text/html'
  print 'Content-Length: %s' % len(response)
  print
  print response


if __name__ == '__main__':
  main()




