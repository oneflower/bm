CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

.PHONY: all
all: bm bmi

bm: ./src/bm.c ./src/bm.h
	$(CC) $(CFLAGS) -o bm ./src/bm.c $(LIBS)

bmi: ./src/bmi.c ./src/bm.h
	$(CC) $(CFLAGS) -o bmi ./src/bmi.c $(LIBS)

bme: ./src/bme.c ./src/bm.h
	$(CC) $(CFLAGS) -o bme ./src/bme.c $(LIBS)

.PHONY: examples
examples: ./examples/fib.bm ./examples/123.bm


./examples/fib.bm: bm ./examples/fib.ebasm
	./bm ./examples/fib.ebasm ./examples/fib.bm

./examples/123.bm: bm ./examples/123.ebasm
	./bm ./examples/123.ebasm ./examples/123.bm
