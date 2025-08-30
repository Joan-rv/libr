NAME=librstd
STATIC=$(NAME).a
DYNAMIC=$(NAME).so

CC?=gcc
CFLAGS?=-Wall -Wextra -Werror -g3
CFLAGS+=-Iinclude
LD?=$(CC)
LDFLAGS?=-lm
AR=ar
RM=rm -f
MKDIR=mkdir -p

SRCDIR=src
OBJDIR=obj
SRCS:=r_printf.c r_fmtprint.c r_args.c r_math.c
SRCS:=$(SRCS:%=$(SRCDIR)/%)
OBJS:=$(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: clean fclean all

all: test static dynamic

test: obj/test.o $(NAME).a
	$(CC) -lm $^ -o $@

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
