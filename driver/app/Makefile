HEADERS = gpio.h

all: gpio.o
	gcc *.o main.c -o main -lwiringPi -lbcm2835

gpio.o: gpio.c $(HEADERS)
	gcc -c gpio.c -o gpio.o

clean:
	rm -f *.o
	rm -f main