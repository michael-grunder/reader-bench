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

typedef struct benchResults {
    size_t cmds;
    long long t1, t2, t3;
} benchResults;

static benchResults benchReader(const char *protofile) {
    redisReader *rdr;
    void **replies;
    size_t i;
    protoFile *pf;

    pf = protoFileOpen(protofile, '^');
    if (pf == NULL) {
        fprintf(stderr, "Error:  Can't open protocol file '%s'", protofile);
        exit(-1);
    }

    rdr = redisReaderCreate();
    replies = malloc(sizeof(*replies) * pf->len);

    long long t1 = usec();
    for (i = 0; i < pf->len; i++) {
        if (redisReaderFeed(rdr, pf->entry[i], sdslen(pf->entry[i])) != REDIS_OK) {
            fprintf(stderr, "Failed to feed readery on entry %zu\n", i);
            fprintf(stderr, "  Error: %s\n", rdr->errstr);
            fprintf(stderr, "--- command ---\n");
            fprintf(stderr, "%s", pf->entry[i]);
            fprintf(stderr, "--- end cmd ---\n");
            exit(-1);
        }
        redisReaderGetReply(rdr, &replies[i]);
    }
    long long t2 = usec();

    for (size_t i = 0; i < pf->len; i++) {
        freeReplyObject(replies[i]);
    }

    long long t3 = usec();

    protoFileFree(pf);
    redisReaderFree(rdr);
    free(replies);

    return (benchResults) {
        .cmds = i,
        .t1 = t1,
        .t2 = t2,
        .t3 = t3
    };
}

int main(int argc, char **argv) {
    benchResults r;

    if (argc < 2) {
        fprintf(stderr, "Error:  Must pass a proto file\n");
        exit(-1);
    }

    r = benchReader(argv[1]);

    static const char *qualifier[] = {
        // "NOFREE",
        "WITHFREE",
    };

    long long timings[] = {
        // r.t2 - r.t1,
        r.t3 - r.t1
    };

    int csv = argc > 2 ? atoi(argv[2]) : 0;

    for (int i = 0; i < sizeof(qualifier)/sizeof(qualifier[0]); i++) {
        long long usecs = timings[i];
        long persec = 1000000.00 * r.cmds / usecs;
        if (csv) {
            printf("%s,%s,%s,%ld,%lld,%ld\n", argv[0], argv[1], qualifier[i], r.cmds, usecs, persec);
        } else {
            printf("%10s %8s %15s %10zu replies in %10lld usec %12ld per sec\n", argv[0],
                argv[1], qualifier[i], r.cmds, usecs, persec);
        }
    }

    return 0;
}
