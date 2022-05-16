CC = gcc
CFLAGS = -c -g
LDFLAGS =  
OBJECTS = main.o fs.o disk.o

run: all
	./program

all: program

program : $(OBJECTS)
	$(CC)  $(LDFLAGS) -o program $(OBJECTS)

main.o : main.c
	$(CC) $(CFLAGS) main.c 

fs.o : fs.c
	$(CC) $(CFLAGS) fs.c

disk.o : disk.c
	$(CC) $(CFLAGS) disk.c