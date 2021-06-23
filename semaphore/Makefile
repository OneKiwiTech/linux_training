HEADERS = list.h fifo.h

all: fifo.o list.o
	gcc *.o main.c -o main -lpthread

fifo.o: fifo.c $(HEADERS)
	gcc -c fifo.c -o fifo.o

list.o: list.o
	gcc -c list.c -o list.o

clean:
	rm -f *.o
	rm -f main