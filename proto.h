#ifndef PROTO_H
#define PROTO_H

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

typedef struct protoFile {
    sds *entry;
    size_t size, len;
} protoFile;

long long usec(void);

protoFile *protoFileCreate(size_t initial);

void protoFileAppend(protoFile *pf, sds cmd);
void protoFileFree(protoFile *pf);
protoFile *protoFileOpen(const char *path, const char delim);

#endif
