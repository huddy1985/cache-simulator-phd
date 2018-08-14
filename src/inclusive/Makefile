include ../../inc.mk

unexport CFLAGS
unexport objs

CFLAGS = -I../../$(INCLUDE) -Werror -Wall
srcs = $(wildcard *.c)
objs = $(patsubst %.c,%.o, $(srcs))

all: $(objs)
	@echo "all"
	mv $(objs) ../../objs

$(objs): %.o : %.c
	@echo $(srcs)
	@echo ".c.o compile"
	$(CC) -c $< -o $@ $(CFLAGS)