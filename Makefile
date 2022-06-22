CC = gcc
CFLAGS = -c -g
LDFLAGS = -no-pie
OBJECTS = validate.o testcase.o hw1.o hw2.o disk.o

run: all
	./program createfs 1

all : program

program : $(OBJECTS)
	$(CC) $(LDFLAGS) -o program $(OBJECTS)

testcase.o : testcase.c
	$(CC) $(CFLAGS) testcase.c

hw2.0 : hw2.c
	$(CC) $(CFLAGS) hw2.c

hw1.o : hw1.c
	$(CC) $(CFLAGS) hw1.c

disk.o : disk.c
	$(CC) $(CFLAGS) disk.c