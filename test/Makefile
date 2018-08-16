DFLAGS := -L../ext-libs -ludis86
CFLAGS := -I../include

test-dis:
	gcc $(CFLAGS) test-dis.c -o test-dis $(DFLAGS)

.PHONY: clean

clean:
	rm -rf test-dis
