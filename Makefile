CC = gcc
CFLAGS = -g -Wall -std=gnu99 -fstack-protector-all

ACEUNIT_LOC = ../aceunit
ACEUNIT_LIB = $(ACEUNIT_LOC)/lib/libaceunit-fork.a
TEST_LOC = tests
VPATH = . $(TEST_LOC)

TESTS = pq queue stack
TEST_BINS = $(addprefix test_,$(TESTS))
TEST_OBJS = $(addsuffix .o,$(TEST_BINS))

TESTCASES = $(addprefix testcases_,$(TESTS))
TESTCASES_SRCS = $(addsuffix .c,$(TESTCASES))
TESTCASES_OBJS = $(addsuffix .o,$(TESTCASES))

BINS = ts ss ss-opt bt bt-opt
OBJS = util.o

.PHONY: all clean test $(TESTS)

all: $(BINS)

test: $(TESTS)

$(OBJS): %.o: %.h

$(BINS): util.o

$(TESTS): CFLAGS += -I $(ACEUNIT_LOC)/include
$(TESTS): %: test_%
	@echo === $@ ===
	@./$^

$(TEST_BINS): test_%: test_%.o testcases_%.o util.o $(ACEUNIT_LIB)

$(TESTCASES_SRCS): testcases_%.c: test_%.o
	$(ACEUNIT_LOC)/bin/aceunit.zsh -s _ $^ >$@

clean:
	$(RM) $(BINS) $(OBJS)
	$(RM) $(TEST_BINS) $(TEST_OBJS) $(TESTCASES_SRCS) $(TESTCASES_OBJS)

clobber: clean
	$(RM) -r *.dSYM
