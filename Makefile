CC = gcc
CFLAGS = -std=c89 -ggdb -Wall -pedantic
OBJDIR = obj
SRCDIR = src

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
HEADERS = $(wildcard $(SRCDIR)/*.h)

.PHONY: all bfc clean

all: $(OBJDIR) bfc

$(OBJDIR):
	mkdir -p $(OBJDIR)

bfc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
	rm -f bfc
