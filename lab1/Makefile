CC = gcc
CFLAGS_DEBUG = -W -Wall -Wextra -std=c11 -g
CFLAGS_RELEASE = -W -Wall -Wextra -std=c11 -O2
.PHONY: clean all
MODE ?= debug
ifeq ($(MODE), debug)
	CFLAGS = $(CFLAGS_DEBUG)
else
	CFLAGS = $(CFLAGS_RELEASE)
endif
all: prog dirwalk
prog: prog.c Makefile
	$(CC) $(CFLAGS) prog.c -o prog
dirwalk: dirwalk.c Makefile
	$(CC) $(CFLAGS) dirwalk.c -o dirwalk
clean:
	rm -f prog dirwalk
