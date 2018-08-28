DFLAGS := -L../ext-libs -ludis86
CFLAGS := -I../include

test-dis:
	gcc $(CFLAGS) test-dis.c -o test-dis $(DFLAGS)

test-capstone:
	gcc -std=c99 -g test-capstone.c -I../include/capstone -I../include -lbfd -lcapstone -o $@
	

.PHONY: clean

clean:
	rm -rf test-dis a.out test-capstone
