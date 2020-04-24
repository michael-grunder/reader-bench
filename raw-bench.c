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
#include "hiredis/read.h"

int getReplyFromReader(redisReader *reader, void **reply) {
    if (redisReaderGetReply(reader,reply) == REDIS_ERR) {
        fprintf(stderr, "Error: %s\n", reader->errstr);
        return REDIS_ERR;
    }
    return REDIS_OK;
}

int getReply(redisReader *r, void **reply) {
    void *aux = NULL;

    /* Try to read pending replies */
    if (getReplyFromReader(r,&aux) == REDIS_ERR)
        return REDIS_ERR;

    *reply = aux;
    return REDIS_OK;
}

FILE *openOrAbort(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error:  Can't open file '%s'\n", filename);
        exit(-1);
    }

    return fp;
}

size_t consumeReplies(redisReader *rdr) {
    size_t count = 0;
    redisReply *reply = NULL;

    do {
        if (getReply(rdr, (void*)&reply) == REDIS_ERR) {
            fprintf(stderr, "Error: %s\n", rdr->errstr);
            exit(-1);
        }

        if (reply) {
            freeReplyObject(reply);
            count++;
        }
    } while (reply);

    return count;
}

size_t benchReader(const char *protofile) {
    redisReader *rdr = redisReaderCreate();
    size_t count = 0;
    char buffer[32768];
    size_t read;
    FILE *fp = openOrAbort(protofile);

    while ((read = fread(buffer, 1, sizeof(buffer), fp)) != 0) {
        redisReaderFeed(rdr, buffer,  read);
        count += consumeReplies(rdr);
    }

    if (read > 0) {
        redisReaderFeed(rdr, buffer, read);
        count += consumeReplies(rdr);
    }

    redisReaderFree(rdr);
    fclose(fp);

    return count;
}

int main(int argc, char **argv) {
    long long t1 = usec();
    size_t count = benchReader(argv[1]);
    long long t2 = usec();

    long long usecs = t2 - t1;
    long persec = 1000000.00 * count / usecs;
    printf("%10s %zu replies in %lld usec %12ld per second\n",
        argv[1], count, usecs, persec);

    return 0;
}
