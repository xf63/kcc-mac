CC=gcc
CFLAGS=-g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

kcc:	$(OBJS)
	$(CC) -o kcc $(OBJS) $(LDFLAGS)

$(OBJS):	kcc.h

test: kcc
	./test.sh

clean:
	rm -f kcc *.o *~ tmp*

.PHONY: test clean