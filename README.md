bitcase
=======

A toy memory database for learning,a lot of things inspired by redis.

### How to build

Bitcase runs on linux now only.First make sure that your system have below software installed:

`gcc`,`make`,`cmake`,`g++`,`wget`,`automake`,`libtool`,`git`,`python`

Ubuntu:

    # get the source
    git clone https://github.com/chow1937/bitcase.git

    # build libuv
    cd bitcase
    make libuv

    # build bitcase
    make

### Test

Here we use python code to send command to server and print the result.

Test code:

    #_*_ coding: utf-8 _*_
    import socket

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('localhost', 8638))
    s.sendall('set\r\n2\r\nname\r\ntony\r\n')
    print s.recv(1024)

Run the test:

    # run bitcase
    cd bitcase
    build/src/bitcase

    # in another shell window
    cd bitcase
    python tests/set.py
    python tests/get.py
    ...
