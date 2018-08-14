include $(CURDIR)/inc.mk

target = cache-simulator
objs = $(wildcard objs/*.o)
objs += main.o
tmp = ./objs

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

$(target):
	@echo $(OBJSDIR)
	rm -rf $(tmp)
	mkdir $(tmp)
	@echo $(type)
	$(MAKE) -C $(file)

.PHONY: clean

clean:
	rm -rf $(objs)