#include "server_eh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <ev.h>
#include "leveldb/c.h"

leveldb_t *DB;
leveldb_cache_t *ldb_cache;
leveldb_options_t *ldb_options;
static struct http_server server;
int n = 1;

int open_db() {
    char* err = NULL;
    ldb_options = leveldb_options_create();
    ldb_cache = leveldb_cache_create_lru(4 << 2048);

    leveldb_options_set_create_if_missing(ldb_options, 1);
    leveldb_options_set_cache(ldb_options, ldb_cache);
    leveldb_options_set_write_buffer_size(ldb_options, 4 << 2048);

    DB = leveldb_open(ldb_options, "./testdb", &err);

    if (err != NULL) {
      fprintf(stderr, "Open fail.\n");
      return(0);
    }
    /* reset error var */
    leveldb_free(err);


    return(1);
}

void close_db() {
    leveldb_close(DB);
    leveldb_options_destroy(ldb_options);
    leveldb_cache_destroy(ldb_cache);
}

int store(char *key, char *value) {
    char *err = NULL;
    leveldb_writebatch_t* wb = leveldb_writebatch_create();
    leveldb_writeoptions_t *woptions;

    woptions = leveldb_writeoptions_create();
    leveldb_writebatch_put(wb, key, strlen(key), value, strlen(value));
    leveldb_writebatch_put(wb, value, strlen(value), key, strlen(key));
    leveldb_write(DB, woptions, wb, &err);

    if (err != NULL) {
        fprintf(stderr, "Write fail.\n");
        return(0);
    }

    leveldb_free(err); err = NULL;
    leveldb_writeoptions_destroy(woptions);
    return(1);
}

char * get(char *key) {
    char *err = NULL;
    size_t read_len;
    char *read;
    leveldb_readoptions_t *roptions;
    roptions = leveldb_readoptions_create();
    read = leveldb_get(DB, roptions, key, strlen(key), &read_len, &err);

    if (err != NULL) {
        fprintf(stderr, "Read fail.\n");
        return(1);
    }

    if (read) {
        char *cnt = malloc(sizeof(char)*(read_len + 1));
        memcpy(cnt, read, read_len);
        cnt[read_len+1] = '\0';
        free(read);
        read = cnt;
    }

    leveldb_free(err); err = NULL;
    return read;
}

char* convert(int number_to_convert) {
    char * retval = malloc(sizeof(char) * 64);

    char base_digits[56] =
    {'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'j', 'k', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K',
    'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', '2', '3', '4', '5', '6', '7', '8',
    '9'};
    int converted_number[64];
    int base = 56;
    int next_digit, index=0;
    /* convert to the indicated base */
    while (number_to_convert != 0)
    {
        converted_number[index] = number_to_convert % base;
        number_to_convert = number_to_convert / base;
        ++index;
    }

    retval[index] = '\0';

    /* now print the result in reverse order */
    --index;  /* back up to last entry in the array */
    for(  ; index>=0; index--) /* go backward through array */
    {
        retval[index] = base_digits[converted_number[index]];
    }
    return retval;
}


void handle_register(struct http_request *request, int fd) {
    char *id = NULL;
    // body of PUT is the url

    // check if in db already
    id = get(request->body);
    if (id != NULL) {
        write(fd, "already found\n", 14);
    } else {
        id = convert(n++);
        store(id, request->body);
    }
    char msg[80];
    sprintf(msg, "shortened url=http://localhost:8000/%s\n", id);
    write(fd, msg, strlen(msg));

    free(id);
}

void handle_redirect(struct http_request *request, int fd) {
    char *id = NULL;

    id = get(&request->url[1]);
    if (id) {
        write(fd, "found", 5);
    } else {
        write(fd, "not found", 9);
    }
    write(fd, "\n", 1);
}

// see http_parser.h
// char DELETE = 0;
// char GET = 1;
// char HEAD = 2;
// char POST = 3;
// char PUT = 4;

void handle_request(struct http_request *request, int fd) {
    if (strcmp(request->url, "/favicon.ico") == 0) {
        return;
    }

    open_db();
    if (request->method == 4) {
        handle_register(request, fd);
    } else if (request->method == 1) {
        handle_redirect(request, fd);
    }
    close(fd);
    close_db();
}


void sigint_handler(int s) {
    struct ev_loop *loop = server.loop;
    ev_io_stop(EV_A_ server.ev_accept);

    // close db
    leveldb_close(DB);

    exit(0);
}

int main(int argc, char **argv) {
    // configure server structures and desired listen address
    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(8000);
    server.listen_addr = &listen_addr;
    server.handle_request = handle_request;

    // ignore SIGPIPE
    struct sigaction on_sigpipe;
    on_sigpipe.sa_handler = SIG_IGN;
    sigemptyset(&on_sigpipe.sa_mask);
    sigaction(SIGPIPE, &on_sigpipe, NULL);

    // handle C-c
    struct sigaction on_sigint;
    on_sigint.sa_handler = sigint_handler;
    sigemptyset(&on_sigint.sa_mask);
    on_sigint.sa_flags = 0;
    sigaction(SIGINT, &on_sigint, NULL);

    // init db stuff


    // start the server
    return http_server_loop(&server);
}
