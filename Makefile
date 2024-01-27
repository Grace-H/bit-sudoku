ACEUNIT_LOC = ../aceunit
ACEUNIT_LIB = $(ACEUNIT_LOC)/lib/libaceunit-fork.a
TEST_LOC = tests

CC = gcc
CFLAGS = -g -pq -Wall -std=gnu99 -fstack-protector-all -I $(ACEUNIT_LOC)/include
OFLAGS = -std=gnu99 -pg

TESTS = pq queue

.PHONY: all clean test_ds $(TESTS)

all: ts ss ss-opt bt bt-opt

test: $(TESTS)

ts: ts.c util.o
	$(CC) $(CFLAGS) -o ts ts.c util.o

ss: ss.c util.o
	$(CC) $(CFLAGS) -o ss ss.c util.o

bt: bt.c util.o
	$(CC) $(CFLAGS) -o bt bt.c util.o

bt-opt: bt-opt.c util.o
	$(CC) $(CFLAGS) -o bt-opt bt-opt.c util.o

ss-opt: ss-opt.c util.o
	$(CC) $(CFLAGS) -o ss-opt ss-opt.c util.o

util.o: util.c
	$(CC) $(OFLAGS) -c util.c

$(TESTS): %: test_%
	@echo === $@ ===
	@./$^

test_pq: util.o test_pq.o testcases_pq.o
	$(CC) util.o test_pq.o testcases_pq.o $(ACEUNIT_LIB) -o test_pq

testcases_pq.c: test_pq.o
	$(ACEUNIT_LOC)/bin/aceunit test_pq.o >testcases_pq.c

test_pq.o: tests/test_pq.c
	$(CC) $(CFLAGS) -c tests/test_pq.c

test_queue: util.o test_queue.o testcases_queue.o
	$(CC) util.o test_queue.o testcases_queue.o $(ACEUNIT_LIB) -o test_queue

testcases_queue.c: test_queue.o
	$(ACEUNIT_LOC)/bin/aceunit test_queue.o >testcases_queue.c

test_queue.o: tests/test_queue.c
	$(CC) $(CFLAGS) -c tests/test_queue.c
clean:
	rm -f -- *.o
	rm -f -- ts ss ss-opt bt bt-opt test_pq test_queue
	rm -f -- testcases_*.c
