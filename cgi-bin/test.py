import datetime


def main():
  response = """
  <html>
    <head>
      <title>Pyton hello!</title>
    </head>
    <body>
      <h1>Hello from Python!</h1>
      <hr>
      <label>Python time:</label> %s
    </body>
  </html>""" % datetime.datetime.now()

  print('Content-Length: %s' % len(response))
  print('Content-Type: text/html')
  print('')
  print(response)


if __name__ == '__main__':
  main()

