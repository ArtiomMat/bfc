CC = gcc
CFLAGS = -std=c89 -ggdb -Wall
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

all: bfc

bfc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) program
