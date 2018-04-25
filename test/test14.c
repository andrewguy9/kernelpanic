#include"kernel/hal.h"
#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/socket.h"
#include"kernel/panic.h"
#include"kernel/mutex.h"

#define PRIME_TAG1 "test13_primes1"
#define PRIME_TAG2 "test13_primes2"

#define STACK_SIZE HAL_MIN_STACK_SIZE

struct MUTEX Kicker;

char CountStack1[STACK_SIZE];
struct THREAD CountThread1;

char CountStack2[STACK_SIZE];
struct THREAD CountThread2;

int Max = 1000000;

#define BUFF_SIZE 32
char Mem1[BUFF_SIZE];
SPACE Buff1 = BufferFromObj(Mem1);
char Mem2[BUFF_SIZE];
SPACE Buff2 = BufferFromObj(Mem2);

struct SOCKET Socket;
struct SOCKET_HANDLE H1;
struct SOCKET_HANDLE H2;

#include<stdio.h>
THREAD_MAIN CountMain;
void * CountMain(void * context) {
  struct SOCKET_HANDLE * socket = (struct SOCKET_HANDLE*) context;
  int v;
  if (MutexLock(&Kicker)) {
    v = 0;
    DATA d = BufferFromObj(v);
    SocketWriteStructBuffer( &d, socket);
  }

  while (1) {
    SPACE s = BufferFromObj(v);
    SocketReadStructBuffer(&s, socket);
    v++;
    DATA d = BufferFromObj(v);
    SocketWriteStructBuffer(&d, socket);
    if (v>Max) {
      SchedulerShutdown();
      return NULL;
    }
  }
  return NULL;
}

int main(int argc, char ** argv)
{
  KernelInit();
  SchedulerStartup();

  MutexInit(&Kicker, FALSE);
  SocketInit(
      &Buff1,
      &Buff2,
      & Socket,
      & H1,
      & H2);
  SchedulerCreateThread(
      &CountThread1,
      100,
      CountStack1,
      STACK_SIZE,
      CountMain,
      &H1,
      TRUE);

  SchedulerCreateThread(
      &CountThread2,
      100,
      CountStack2,
      STACK_SIZE,
      CountMain,
      &H2,
      TRUE);

  KernelStart();

  return 0;
}
