include $(CURDIR)/inc.mk

target = cache-simulator
inclusive_obj = $(patsubst %.c,%.o, $(wildcard $(INCLUSIVE_SRC_DIR)/*.c))
cfg_obj = $(patsubst %.c,%.o, $(wildcard $(CFG_PARSER)/*.c))
srcs = main.c
objs = main.o

unexport CFLAGS
CFLAGS := -I./include -std=c99

test: $(target)
	./$(target)

$(target): $(inclusive_obj) $(cfg_obj) $(objs) 
	$(CC) $(inclusive_obj) $(cfg_obj) $(objs) -o $@

$(objs):
	$(CC) -c $(srcs) -o $@ $(CFLAGS)

$(inclusive_obj):
	$(MAKE) -C $(INCLUSIVE_SRC_DIR)

$(cfg_obj):
	$(MAKE) -C $(CFG_PARSER)

.PHONY: clean

clean:
	rm -rf $(objs) $(inclusive_obj) $(cfg_obj) $(target) $(tmp)
