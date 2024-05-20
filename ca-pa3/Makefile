TARGET	= pipesim
CFLAGS	= -c -g

all: pipesim

pipesim: pa3.o main.o
	gcc $^ -o $@

%.o: %.c
	gcc $(CFLAGS) $^ -o $@

.PHONY: cscope
cscope:
	cscope -b -R
	ctags *

.PHONY: clean
clean:
	rm -rf $(TARGET) *.o *.dSYM cscope.out tags
