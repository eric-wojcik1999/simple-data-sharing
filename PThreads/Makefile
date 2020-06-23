CC = gcc
CFLAGS = -Wall -ansi -pedantic -std=gnu99
LIB = -pthread
OBJ = pthread.o fileio.o
EXEC = pt

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LIB)

pthread.o: pthread.c pthread.h
	$(CC) -c pthread.c $(CFLAGS)

fileio.o: fileio.c
	$(CC) -c fileio.c $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJ);
