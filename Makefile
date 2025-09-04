NAME=librstd
STATIC=$(NAME).a
DYNAMIC=$(NAME).so

CC?=gcc
CFLAGS?=-Wall -Wextra -pedantic -g3 -std=c99 -fsanitize=address,undefined
CFLAGS+=-Iinclude -MMD -MP -fPIC
LD=$(CC)
LDFLAGS?=
LDFLAGS+=-lm -fsanitize=address,undefined
AR=ar
RM=rm -f
MKDIR=mkdir -p

SRCDIR=src
OBJDIR=obj
SRCS:=r_printf.c r_fmtprint.c r_args.c r_math.c r_alloc.c
SRCS:=$(SRCS:%=$(SRCDIR)/%)
OBJS:=$(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS:=$(OBJS:.o=.d)

.PHONY: clean fclean all

all: test static dynamic

test: obj/test.o $(NAME).a
	$(LD) $(LDFLAGS) -lm $^ -o $@

static: $(NAME).a

dynamic: $(NAME).so

$(NAME).a: $(OBJS)
	$(AR) rcs $@ $^

$(NAME).so: $(OBJS)
	$(LD) $(LDFLAGS) -shared $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	$(MKDIR) $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(OBJS) $(NAME).a $(NAME).so test

-include $(DEPS)
