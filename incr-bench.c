#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>

#include "proto.h"
#include "hiredis/hiredis.h"

#define RESP "*2\r\n$4\r\nINCR\r\n$7\r\nCOUNTER\r\n"

int main(int argc, char **argv) {
    redisContext *c;
    redisReply *reply;
    size_t count;

    count = argc > 1 ? atoi(argv[1]) : 100000;

    c = redisConnectUnix("/tmp/redis-9999.sock");
    if (c == NULL || c->err) {
        fprintf(stderr, "Error:  Can't connect\n");
        exit(-1);
    }

    for (int i = 0; i < count; i++) {
        redisAppendFormattedCommand(c, RESP, sizeof(RESP) - 1);
    }

    long long t1 = usec();
    for (int i = 0; i < count; i++) {
        if (redisGetReply(c, (void*)&reply) == REDIS_ERR) {
            fprintf(stderr, "Error:  %s\n", c->errstr);
            exit(-1);
        }
        freeReplyObject(reply);
    }
    long long t2 = usec();

    long long usecs = t2 - t1;
    long persec = 1000000.00 * count / usecs;
    printf("%zu replies in %lld usec %12ld per second\n",
        count, usecs, persec);

    return 0;
}
