CFLAGS=-std=c11 -g -static

mcc:	mcc.c

test: mcc
	../auto_test/auto_test.sh

clean:
	rm -f mcc *.o *~ tmp*

.PHONY: test clean
