CC=gcc
CFLAGS=-D_POSIX_C_SOURCE=200809L -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic

.PHONY: clean

export CHILD_PATH=./child
export LC_COLLATE=С

all: parent child run

parent: parent.c
	$(CC) $(CFLAGS) -o parent parent.c

child: child.c
	$(CC) $(CFLAGS) -o child child.c

run: parent
	./parent env.txt
clean:
	rm parent child
