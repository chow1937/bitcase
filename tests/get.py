#_*_ coding: utf-8 _*_
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 8638))
s.sendall('get\r\n1\r\nname\r\n')
print s.recv(1024)
