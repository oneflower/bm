CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

bm: bm.c
	$(CC) $(CFLAGS) -o bm bm.c $(LIBS)



