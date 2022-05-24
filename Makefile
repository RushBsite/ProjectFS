CC = gcc
CFLAGS = -c -g
LDFLAGS = -no-pie
OBJECTS = validate.o testcase.o fs.o disk.o

run: all
	./hw1

all : hw1

hw1 : $(OBJECTS)
	$(CC) $(LDFLAGS) -o hw1 $(OBJECTS)

testcase.o : testcase.c
	$(CC) $(CFLAGS) testcase.c 

fs.o : fs.c
	$(CC) $(CFLAGS) fs.c

disk.o : disk.c
	$(CC) $(CFLAGS) disk.c