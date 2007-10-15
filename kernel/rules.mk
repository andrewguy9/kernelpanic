######################
#kernel
kernel/startup.o: kernel/startup.c kernel/startup.h

kernel/timer.o: kernel/timer.c kernel/timer.h

kernel/hal.o: kernel/hal.c kernel/hal.h

kernel/scheduler.o: kernel/scheduler.c kernel/scheduler.h

kernel/mutex.o: kernel/mutex.c kernel/mutex.h

kernel/semaphore.o: kernel/semaphore.c kernel/semaphore.h

kernel/resource.o: kernel/resource.c kernel/resource.h

kernel/panic.o: kernel/panic.c kernel/panic.h
