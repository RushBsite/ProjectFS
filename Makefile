CC = gcc
CFLAGS = -c -g
LDFLAGS = -no-pie
OBJECTS = validate.o testcase.o fs.o disk.o

run: all
	./program

all: program

program : $(OBJECTS)
	$(CC) $(LDFLAGS) -o program $(OBJECTS)

testcase.o : testcase.c
	$(CC) $(CFLAGS) testcase.c 

fs.o : fs.c
	$(CC) $(CFLAGS) fs.c

disk.o : disk.c
	$(CC) $(CFLAGS) disk.c