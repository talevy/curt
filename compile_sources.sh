#!/bin/sh

set -e;

# symlink libtoolize for mac os x
unamestr=`uname`
if [[ ("$unamestr" == 'Darwin') && ! (-f /usr/local/bin/libtoolize)]]
then
    ln -s /usr/local/bin/glibtoolize /usr/local/bin/libtoolize
fi

(
        svn checkout http://snappy.googlecode.com/svn/trunk/ snappy-read-only;
        cd snappy-read-only;
        ./autogen.sh;
        ./configure --enable-shared=no --enable-static=yes;
        make clean;
        make CXXFLAGS='-g -O2 -fPIC';
)

(
        git clone https://code.google.com/p/leveldb/ || (cd leveldb; git pull);
        cd leveldb;
        make clean;
        make libleveldb.a LDFLAGS='-L../snappy-read-only/.libs/ -Bstatic -lsnappy' OPT='-fPIC -O2 -DNDEBUG -DSNAPPY -I../snappy-read-only' SNAPPY_CFLAGS=''
)

(
        git clone https://github.com/joyent/http-parser.git || (cd http-parser; git pull);
        cd http-parser;
        make clean;
        make package;
)
