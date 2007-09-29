all: linkedlist.o heap.o tags #linktest.out

tags: 
	ctags -R .
linkedlist.o: linkedlist.h linkedlist.c link.h
	avr-gcc -g -c linkedlist.c -o linkedlist.o

heap.o: heap.c heap.h link.h
	avr-gcc -g -c heap.c -o heap.o

#linktest.out: heap.o linkedlist.o linktest.c
#	avr-gcc -g linktest.c heap.o linkedlist.o -o linktest.out

clean:
	rm -rf *.o *.out *~ tags 
