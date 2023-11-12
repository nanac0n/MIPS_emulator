TARGET	= pa1
CFLAGS	= -g

all: pa1

pa1: pa1.c
	gcc $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -rf pa1 *.o pa1.dSYM

.PHONY: test-r
test-r: pa1 testcases/r-format
	./$< testcases/r-format

.PHONY: pa1 test-shifts
test-shifts: pa1 testcases/shifts
	./$< testcases/shifts

.PHONY: pa1 test-i
test-i: pa1 testcases/i-format
	./$< testcases/i-format

.PHONY: test-all
test-all: test-r test-shifts test-i
