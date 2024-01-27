CFLAGS = -g -pg -Wall -std=gnu99 -fstack-protector-all -I $(ACEUNIT_LOC)/include

ACEUNIT_LOC = ../aceunit
ACEUNIT_LIB = $(ACEUNIT_LOC)/lib/libaceunit-fork.a
TEST_LOC = tests
TESTS = pq queue

BINS = ts ss ss-opt bt bt-opt
OBJS = util.o

.PHONY: all clean test_ds $(TESTS)

all: $(BINS)

test: $(TESTS)

$(OBJS): %.o: %.h

$(BINS): util.o

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
	$(RM) $(BINS) $(OBJS)
	rm -f -- test_pq test_queue
	rm -f -- testcases_*.c
