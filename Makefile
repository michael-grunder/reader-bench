BIN=bench
OBJ=alloc.o async.o dict.o hiredis.o net.o read.o sds.o sockcompat.o proto.o
OPT?=-O2
CFLAGS+=-Wall -pedantic -std=gnu99 -ggdb3

all: bench raw-bench incr-bench fail scratch

bench: $(OBJ) bench.o
	$(CC) $(CFLAGS) -o $@ $^

raw-bench: $(OBJ) raw-bench.o
	$(CC) $(CFLAGS) -o $@ $^

incr-bench: $(OBJ) incr-bench.o
	$(CC) $(CFLAGS) -o $@ $^

fail: $(OBJ) fail.o
	$(CC) $(CFLAGS) -o $@ $^

scratch: $(OBJ) scratch.o
	$(CC) $(CFLAGS) -o $@ $^
	OPT=-O0 $(MAKE)

# Generate with make dep
alloc.o: hiredis/alloc.c hiredis/fmacros.h hiredis/alloc.h
async.o: hiredis/async.c hiredis/fmacros.h hiredis/alloc.h hiredis/async.h \
	hiredis/hiredis.h hiredis/read.h hiredis/sds.h hiredis/net.h hiredis/dict.c \
	hiredis/dict.h hiredis/win32.h hiredis/async_private.h
bench.o: bench.c hiredis/hiredis.h hiredis/read.h hiredis/sds.h hiredis/alloc.h \
	hiredis/read.h
dict.o: hiredis/dict.c hiredis/fmacros.h hiredis/alloc.h hiredis/dict.h
fail.o: fail.c hiredis/hiredis.h hiredis/read.h hiredis/sds.h \
	hiredis/alloc.h hiredis/read.h
hiredis.o: hiredis/hiredis.c hiredis/fmacros.h hiredis/hiredis.h \
	hiredis/read.h hiredis/sds.h hiredis/alloc.h hiredis/net.h \
	hiredis/async.h hiredis/win32.h
incr-bench.o: incr-bench.c proto.h hiredis/hiredis.h hiredis/read.h \
	hiredis/sds.h hiredis/alloc.h hiredis/read.h
net.o: hiredis/net.c hiredis/fmacros.h hiredis/net.h hiredis/hiredis.h \
	hiredis/read.h hiredis/sds.h hiredis/alloc.h hiredis/sockcompat.h \
	hiredis/win32.h
proto.o: proto.c proto.h hiredis/hiredis.h hiredis/read.h hiredis/sds.h \
	hiredis/alloc.h hiredis/read.h
read.o: hiredis/read.c hiredis/fmacros.h hiredis/read.h hiredis/sds.h \
	hiredis/win32.h
raw-bench.o: raw-bench.c proto.h hiredis/hiredis.h hiredis/read.h \
	hiredis/sds.h hiredis/alloc.h hiredis/read.h
scratch.o: scratch.c hiredis/hiredis.h hiredis/read.h hiredis/sds.h \
 	hiredis/alloc.h hiredis/read.h
sds.o: hiredis/sds.c hiredis/sds.h hiredis/sdsalloc.h
sockcompat.o: hiredis/sockcompat.c hiredis/sockcompat.h

%.o:
	$(CC) $(OPT) $(CFLAGS) -c $<

dep:
	ls *.c hiredis/*.c |grep -Ev "test|ssl" |xargs cc -MM

clean:
	rm -f *.o bench bench-* raw-bench-* incr-bench-* fail scratch

.PHONY: all clean
