###################
#utilities
utils/$(TARGET)_linkedlist.o: utils/linkedlist.c utils/linkedlist.h

utils/$(TARGET)_heap.o: utils/heap.c utils/heap.h

utils/$(TARGET)_ringbuffer.o: utils/ringbuffer.c utils/ringbuffer.h

utils/$(TARGET)_flags.o: utils/flags.c utils/flags.h

utils/$(TARGET)_bham.o: utils/bham.c utils/bham.h

###################
#tests
utils/linktest.o: utils/linktest.c 

utils/ringtest.o: utils/ringtest.c

utils/flagtest.o: utils/flagtest.c

utils/bhamtest.o: utils/bhamtest.c
