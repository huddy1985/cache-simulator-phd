include $(CURDIR)/inc.mk

target = cache-simulator
cache_objs = $(wildcard objs/*.o)
srcs = main.c
objs = main.o
tmp = ./objs

unexport CFLAGS
CFLAGS := -I./include
cc = gcc

# compile inclusive cache hierachy
ifeq ($(type),INC)
file := src/inclusive/
# compile exclusive cache hierachy
else ifdef ($(type),EXC)
file = exclusive
# compile non-inclusive-non-exclusive cache hierachy
else
file = NINE
endif

$(target): $(objs)
	@echo $(OBJSDIR)
	rm -rf $(tmp)
	mkdir $(tmp)
	@echo $(type)
	$(MAKE) -C $(file)
	$(MAKE) -C cfg-parser
	$(cc) $(cache_objs) $(objs) -o $@

$(objs):
	$(CC) -c $(srcs) -o $@ $(CFLAGS)

.PHONY: clean

clean:
	rm -rf $(objs) $(cache_objs) $(target)
