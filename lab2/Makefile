CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic
all: lab2
lab2: parent.o child.o
    $(CC) $(CFLAGS) parent.o child.o -o lab2
parent.o: parent.c
    $(CC) $(CFLAGS) -c parent.c -o parent.o
child.o: child.c
    $(CC) $(CFLAGS) -c child.c -o child.o
clean:
    rm -f lab2 *.o
