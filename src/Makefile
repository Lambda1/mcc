CFLAGS=-std=c99 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mcc:	$(OBJS)
			$(CC) -o mcc $(OBJS) $(LDFLAGS)

$(OBJS): mcc.h

test: mcc
	../auto_test/auto_test.sh

clean:
	rm -f mcc *.o *~ tmp* a.out

.PHONY: test clean
