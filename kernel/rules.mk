######################
#kernel
kernel/$(TARGET)_startup.o: kernel/startup.c kernel/startup.h

kernel/$(TARGET)_timer.o: kernel/timer.c kernel/timer.h

kernel/$(TARGET)_time.o: kernel/time.c kernel/time.h

kernel/$(TARGET)_hal.o: kernel/hal.c kernel/hal.h hal/avr_hal.c hal/avr_hal.h hal/unix_hal.c hal/unix_hal.h

kernel/$(TARGET)_scheduler.o: kernel/scheduler.c kernel/scheduler.h

kernel/$(TARGET)_mutex.o: kernel/mutex.c kernel/mutex.h

kernel/$(TARGET)_semaphore.o: kernel/semaphore.c kernel/semaphore.h

kernel/$(TARGET)_resource.o: kernel/resource.c kernel/resource.h

kernel/$(TARGET)_panic.o: kernel/panic.c kernel/panic.h

kernel/$(TARGET)_sleep.o: kernel/sleep.c kernel/sleep.h

kernel/$(TARGET)_isr.o: kernel/isr.c kernel/isr.h

kernel/$(TARGET)_softinterrupt.o: kernel/softinterrupt.c kernel/softinterrupt.h

kernel/$(TARGET)_critinterrupt.o: kernel/critinterrupt.c kernel/critinterrupt.h

kernel/$(TARGET)_socket.o: kernel/socket.c kernel/socket.h

kernel/$(TARGET)_pipe.o: kernel/pipe.c kernel/pipe.h

kernel/$(TARGET)_locking.o: kernel/locking.c kernel/locking.h

kernel/$(TARGET)_worker.o: kernel/worker.c kernel/worker.h

kernel/$(TARGET)_gather.o: kernel/gather.c kernel/gather.h

kernel/$(TARGET)_context.o: kernel/context.c kernel/context.h

kernel/$(TARGET)_watchdog.o: kernel/watchdog.c kernel/watchdog.h

kernel/$(TARGET)_signal.o: kernel/signal.c kernel/signal.h

kernel/$(TARGET)_handler.o: kernel/handler.c kernel/handler.h

kernel/$(TARGET)_serial.o: kernel/serial.c kernel/serial.h

kernel/$(TARGET)_generation.o: kernel/generation.c kernel/generation.h

kernel/$(TARGET)_coroutine.o: kernel/coroutine.c kernel/coroutine.h
