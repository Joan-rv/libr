CC=gcc
CFLAGS=-Wall -Wextra -Werror -g -I.
AR=ar
RM=rm -f
OBJ=r_printf.o

.PHONY: clean

all: test lib

test: test.o libr.a
	$(CC) $^ -o $@

lib: libr.a libr.so

libr.a: $(OBJ)
	$(AR) rcs $@ $^

libr.so: $(OBJ)
	$(CC) -shared $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) *.o *.so
