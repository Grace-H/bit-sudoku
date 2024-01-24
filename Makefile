ACEUNIT_LOC = ../aceunit
ACEUNIT_LIB = $(ACEUNIT_LOC)/lib/libaceunit-fork.a
TEST_LOC = tests

CC = gcc
CFLAGS = -g -Wall -std=gnu99 -fstack-protector-all -I $(ACEUNIT_LOC)/include
OFLAGS = -std=gnu99 -pg

.PHONY: all clean test_ds

all: ts ss ss-opt bt bt-opt

ts: ts.c util.o
	$(CC) $(CFLAGS) -o ts ts.c util.o

ss: ss.c util.o
	$(CC) $(CFLAGS) -o ss ss.c util.o

bt: bt.c util.o
	$(CC) $(CFLAGS) -o bt bt.c util.o

bt-opt: bt-opt.c util.o
	$(CC) $(CFLAGS) -o bt-opt bt-opt.c util.o

ss-opt: ss-opt.c util.o
	$(CC) $(OFLAGS) -o ss-opt ss-opt.c util.o

util.o: util.c
	$(CC) $(OFLAGS) -c util.c

test_ds: test_pq
	./$^

test_pq: util.o test_pq.o testcases.o
	$(CC) util.o test_pq.o testcases.o $(ACEUNIT_LIB) -o test_pq

testcases.c: test_pq.o
	$(ACEUNIT_LOC)/bin/aceunit test_pq.o >testcases.c

test_pq.o: tests/test_pq.c
	$(CC) $(CFLAGS) -c tests/test_pq.c

clean:
	rm -f *.o
	rm -f ts ss ss-opt bt bt-opt test_pq
	rm testcases.c
