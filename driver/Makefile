HEADERS = gpio.h

all: gpio.o
	gcc *.o segment_display.c -o main -lpthread

gpio.o: gpio.c $(HEADERS)
	gcc -c gpio.c -o gpio.o

clean:
	rm -f *.o
	rm -f main