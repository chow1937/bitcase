#_*_ coding: utf-8 _*_
import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 8638))
s.sendall('set\r\n2\r\nname\r\ntony\r\n')
print s.recv(1024)
