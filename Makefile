CC=gcc
CFLAGS=-fPIC -Wall -Wextra -Werror -g -I.
AR=ar
RM=rm -f
OBJ=r_printf.o r_fmtprint.o r_args.o r_math.o

.PHONY: clean all

all: test lib

test: test.o libr.a
	$(CC) -lm $^ -o $@

lib: libr.a libr.so

libr.a: $(OBJ)
	$(AR) rcs $@ $^

libr.so: $(OBJ)
	$(CC) -shared $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) *.o *.a *.so test
