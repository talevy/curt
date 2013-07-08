curt
====

[in progress] a very shrt url shrtnr


##### Borrows heavily from:

1. [leveldb](https://code.google.com/p/leveldb/)
2. [http-parser](https://github.com/joyent/http-parser)
3. [webserver-libev-httpparser](https://github.com/dexgeh/webserver-libev-httpparser)

Dependencies
------------
* automake
* pkg-config
* subversion
* git
* libtool

How to run.
-----------

```bash
./compile-sources.sh
make
./curt
```

How to shorten.
---------------
```bash
curl -X PUT --data "http://myfavoritesite.com" http://127.0.0.1:8000

shortened url=http://localhost:8000/b
```

How to retrieve.
----------------
```bash
curl http://127.0.0.1:8000/b

found
```

It is not very useful for now, but its just cosmetic sugar to make it actually redirect urls.
