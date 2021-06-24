HEADERS = list.h fifo.h

all: fifo.o my_list.o
	gcc *.o main.c -o main -lpthread

fifo.o: fifo.c $(HEADERS)
	gcc -c fifo.c -o fifo.o

list.o: my_list.c
	gcc -c my_list.c -o my_list.o

clean:
	rm -f *.o
	rm -f main