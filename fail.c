#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <getopt.h>

#include "proto.h"
#include "hiredis/hiredis.h"
#include "hiredis/read.h"

typedef struct runOpts {
    const char *file;
    int seed;
    int limit;
    int renew;
    int pct;
    int quiet;
} runOpts;

redisReader *getReader(size_t on, size_t of, int quiet, int *fails) {
    redisReader *rdr;

    do {
        rdr = redisReaderCreate();
        if (rdr == NULL) {
            *fails+=1;
            if (!quiet)
                fprintf(stderr, "[%zu/%zu]: CREATE: (rdr == NULL)\n", on, of);
        }
    } while (rdr == NULL);

    return rdr;
}

static int _rndRange(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

int _pctFail(int pct) {
    if (_rndRange(1, 100) <= pct)
        return 1;

    return 0;
}

void printUsage(const char *prog) {
    fprintf(stderr, "Usage %s [OPTIONS] protofile\n", prog);
    fprintf(stderr, "  -s seed\n");
    fprintf(stderr, "  -p fail percentage\n");
    fprintf(stderr, "  -p limit\n");
    fprintf(stderr, "  -r renew reader each iteration\n");
    fprintf(stderr, "  -q quiet mode\n");
    fprintf(stderr, "  -h this message\n");
    exit(0);
}

runOpts parseArgs(int argc, char **argv) {
    runOpts ro = {0};
    int opt;

    ro.seed = -1;
    ro.limit = -1;
    ro.pct = 5;

    while ((opt = getopt(argc, argv, "rqhs:p:l:")) != -1) {
        switch (opt) {
            case 's':
                ro.seed = atoi(optarg);
                break;
            case 'p':
                ro.pct = atoi(optarg);
                break;
            case 'l':
                ro.limit = atoi(optarg);
                break;
            case 'r':
                ro.renew = 1;
                break;
            case 'q':
                ro.quiet = 1;
                break;
            case 'h':
                printUsage(argv[0]);
            default:
                fprintf(stderr, "Error:  Don't know option '%c'\n", opt);
        }
    }

    if (argc <= optind || !*argv[optind]) {
        fprintf(stderr, "Error:  Must pass an input file!\n");
        exit(-1);
    }

    ro.file = argv[optind];
    if (ro.seed == -1) ro.seed = time(NULL);

    return ro;
}

void setPctFail(int i) {
}

int main(int argc, char **argv) {
    int failures = 0, cfails = 0, ffails = 0, rfails = 0;
    size_t i;
    protoFile *pf;
    redisReader *rdr;
    redisReply *reply;

    runOpts ro = parseArgs(argc, argv);

    printf("File: %s, pct: %d, seed: %u, renew: %d, quiet: %d\n",
           ro.file, ro.pct, ro.seed, ro.renew, ro.quiet);

    srand(ro.seed);
    setPctFail(ro.pct);

    pf = protoFileOpen(ro.file, '^');
    if (pf == NULL) {
        fprintf(stderr, "Error:  Can't open proto file '%s'\n", ro.file);
        exit(-1);
    }

    rdr = getReader(0, pf->len, ro.quiet, &cfails);
    for(i = 0; i < pf->len && (ro.limit == -1 || i < ro.limit); i++) {
        if (redisReaderFeed(rdr, pf->entry[i], sdslen(pf->entry[i])) != REDIS_OK) {
            if (!ro.quiet)
                fprintf(stderr, "[%zu/%zu]:   FEED: '%s'\n", i, pf->len, rdr->errstr);
            redisReaderFree(rdr);
            rdr = getReader(i, pf->len, ro.quiet, &cfails);
            ffails++;
        }

        if (redisReaderGetReply(rdr, (void**)&reply) == REDIS_ERR) {
            if (!ro.quiet)
                fprintf(stderr, "[%zu/%zu]:  REPLY: %s\n", i, pf->len, rdr->errstr);
            redisReaderFree(rdr);
            rdr = getReader(i, pf->len, ro.quiet, &cfails);
            rfails++;
        }

        if (reply)
            freeReplyObject(reply);
        if (ro.renew) {
            redisReaderFree(rdr);
            rdr = getReader(i, pf->len, ro.quiet, &cfails);
        }
    }

    failures = cfails + ffails + rfails;
    fprintf(stderr, "[%zu/%zu]: FINAL:  %d failures (%d create, %d feed, %d reply)\n",
            i, pf->len, failures, cfails, ffails, rfails);

    protoFileFree(pf);
    redisReaderFree(rdr);

    return 0;
}
