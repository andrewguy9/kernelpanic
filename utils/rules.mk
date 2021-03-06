###################
#utilities
utils/$(TARGET)_linkedlist.o: utils/linkedlist.c utils/linkedlist.h

utils/$(TARGET)_heap.o: utils/heap.c utils/heap.h

utils/$(TARGET)_ringbuffer.o: utils/ringbuffer.c utils/ringbuffer.h

utils/$(TARGET)_bitmap.o: utils/bitmap.c utils/bitmap.h

utils/$(TARGET)_bham.o: utils/bham.c utils/bham.h

utils/$(TARGET)_avl.o: utils/avl.c utils/avl.h

utils/$(TARGET)_types.o: utils/types.c utils/types.h

utils/$(TARGET)_atomiclist.o: utils/atomiclist.c utils/atomiclist.h

utils/$(TARGET)_counter.o: utils/counter.c utils/counter.h

utils/$(TARGET)_counter.o: utils/counter.c utils/counter.h

utils/$(TARGET)_primes.o: utils/primes.c utils/primes.h
###################
#tests
utils/$(TARGET)_linktest.o: utils/linktest.c 

utils/$(TARGET)_treetest.o: utils/treetest.c

utils/$(TARGET)_ringtest.o: utils/ringtest.c

utils/$(TARGET)_bitmaptest.o: utils/bitmaptest.c

utils/$(TARGET)_bhamtest.o: utils/bhamtest.c

utils/$(TARGET)_primestest.o: utils/primestest.c
