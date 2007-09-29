all: timer.o

timer.o: timer.c timer.h
	avr-gcc -Wall -mmcu=atmega128 -O2 -c -g timer.c -o timer.o

tags: 
	ctags -R .

clean:
	rm -rf *.o *.out *~ tags 
