#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>

#include "hiredis/hiredis.h"
#include "hiredis/read.h"

int main(int argc, char **argv) {
    redisReader *rdr = redisReaderCreate();
    redisReply *reply;

    sds ival = sdsnew(":1234567\r\n");
    redisReaderFeed(rdr, ival, sdslen(ival));

    redisReaderGetReply(rdr, (void*)&reply);
    redisReaderFree(rdr);
    if (reply && reply->type == REDIS_REPLY_INTEGER) {
        printf("Int: %lld\n", reply->integer);
    }
    freeReplyObject(reply);

    return 0;
}
