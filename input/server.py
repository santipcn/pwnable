import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#now connect to the web server on port 80
# - the normal http port
s.connect(("127.0.0.1", 1337))
s.send("\xde\xad\xbe\xef")
