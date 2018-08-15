include ../../inc.mk

unexport CFLAGS
unexport objs

CFLAGS = -I../../$(INCLUDE) -Werror -Wall
srcs = $(wildcard *.c)
objs = $(patsubst %.c,%.o, $(srcs))

all: $(objs)

$(objs): %.o : %.c
	$(CC) -c $< -o $@ $(CFLAGS)
