#include "proto.h"

long long usec(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

protoFile *protoFileCreate(size_t initial) {
    protoFile *pf = malloc(sizeof(*pf));
    pf->entry = malloc(sizeof(*pf->entry) * initial);
    pf->size = initial;
    pf->len = 0;
    return pf;
}

void protoFileAppend(protoFile *pf, sds cmd) {
    if (pf->len == pf->size) {
        pf->size *= 2;
        pf->entry = realloc(pf->entry, pf->size * sizeof(*pf->entry));
    }

    pf->entry[pf->len++] = cmd;
}

void protoFileFree(protoFile *pf) {
    for (size_t i = 0; i < pf->len; i++)
        sdsfree(pf->entry[i]);

    free(pf->entry);
    free(pf);
}

protoFile *protoFileOpen(const char *path, const char delim) {
    char buffer[32], *p, *pos;
    ssize_t read;
    protoFile *pf;
    FILE *fp;

    if ((pf = protoFileCreate(32)) == NULL || (fp = fopen(path, "r")) == NULL) {
        protoFileFree(pf);
        return NULL;
    }

    sds cmd = sdsempty();
    while ((read = fread(buffer, 1, sizeof(buffer), fp)) != 0) {
        pos = buffer;
        while (read > 0 && (p = memchr(pos, delim, read)) != NULL) {
            cmd = sdscatlen(cmd, pos, p - pos);
            protoFileAppend(pf, cmd);
            cmd = sdsempty();
            read -= p - pos + 1;
            pos = p + 1;
        }

        if (read > 0) {
            cmd = sdscatlen(cmd, pos, read);
        }
    }

    if (sdslen(cmd)) {
        protoFileAppend(pf, cmd);
    }

    fclose(fp);
    return pf;
}

