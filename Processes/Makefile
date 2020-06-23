GCC = gcc
CFLAGS = -Wall -ansi -pedantic -pthread -std=gnu99 
OBJ = processes.o fileio.o
LIBS = -lrt 
EXEC = pro

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LIBS)

processes.o: processes.c processes.h
	$(CC) -c processes.c $(CFLAGS)

fileio.o: fileio.c
	$(CC) -c fileio.c $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJ);
