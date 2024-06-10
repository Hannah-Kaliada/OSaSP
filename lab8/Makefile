CC=gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic -pthread

SRCS_SERVER=server.c
SRCS_CLIENT=client.c

OBJS_SERVER=$(SRCS_SERVER:.c=.o)
OBJS_CLIENT=$(SRCS_CLIENT:.c=.o)

all: server client

server: $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(OBJS_SERVER) -o $@

client: $(OBJS_CLIENT)
	$(CC) $(CFLAGS) $(OBJS_CLIENT) -o $@

clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) server client
