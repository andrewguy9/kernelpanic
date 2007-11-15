###################
#utilities
utils/$(TARGET)_linkedlist.o: utils/linkedlist.c utils/linkedlist.h

utils/$(TARGET)_heap.o: utils/heap.c utils/heap.h

utils/$(TARGET)_ringbuffer.o: utils/ringbuffer.c utils/ringbuffer.h

###################
#tests
utils/linktest.o: utils/linktest.c 
