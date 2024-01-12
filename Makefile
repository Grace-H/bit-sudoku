CC = gcc
CFLAGS = -g -Wall -std=gnu99 -fstack-protector-all
OFLAGS = -std=gnu99

.PHONY: all clean

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

clean:
	rm -f *.o
	rm -f ts ss ss-opt bt bt-opt
