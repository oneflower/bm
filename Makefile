CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

all: bm bmi

bm: bm.c bm.h
	$(CC) $(CFLAGS) -o bm bm.c $(LIBS)

bmi: bmi.c bm.h
	$(CC) $(CFLAGS) -o bmi bmi.c $(LIBS)

