NAME=curt
CC=gcc
CFLAGS=-std=gnu99 -Wall -I./leveldb/include -fPIC -I./http-parser \
	   -g -Wno-unused -fno-strict-aliasing -Wstrict-prototypes \
	   -Wmissing-declarations
LDFLAGS= -L./leveldb -Bstatic -lleveldb -L./snappy-read-only/.libs/ \
		 -Bstatic -lsnappy -lev ./http-parser/libhttp_parser.a -lpthread
SOURCES= server_eh.c main.c
OBJS= server_eh.o main.o

all: clean $(NAME)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LDFLAGS) -lstdc++

compile_sources:
	./compile_sources.sh

clean_sources:
	rm -rf leveldb snappy-read-only http-parser

clean:
	rm -rf $(NAME) *.o testdb

run: all
	./$(NAME)

