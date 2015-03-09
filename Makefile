CC=gcc
CFLAGS=-I.
DEPS = test_ipc.h
OBJ = test_ipc.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test_ipc: $(OBJ)
	gcc -pthread -lrt -o $@ $^ $(CFLAGS)
