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

all: prog

prog: prog.c makefile
    $(CC) $(CFLAGS) prog.c -o prog

clean:
    rm -f prog
