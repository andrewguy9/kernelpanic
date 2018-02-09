#include"kernel/hal.h"
#include"kernel/startup.h"
#include"kernel/scheduler.h"
#include"kernel/socket.h"
#include"kernel/panic.h"

#define PRIME_TAG1 "test13_primes1"
#define PRIME_TAG2 "test13_primes2"

#define STACK_SIZE HAL_MIN_STACK_SIZE

char CountStack1[STACK_SIZE];
struct THREAD CountThread1;

char CountStack2[STACK_SIZE];
struct THREAD CountThread2;

int Max = 1000000;

#define BUFF_SIZE 32
char Buff1[BUFF_SIZE];
char Buff2[BUFF_SIZE];

struct SOCKET Socket;
struct SOCKET_HANDLE H1;
struct SOCKET_HANDLE H2;

#include<stdio.h>
THREAD_MAIN CountMain;
void CountMain(void * context) {
  struct SOCKET_HANDLE * socket = (struct SOCKET_HANDLE*) context;
  int v;
  //TODO MAKE ONLY HAPPEN ONCE.
  SocketWriteStruct((char*) &v, sizeof(v), socket);

  while (1) {
    SocketReadStruct((char*) &v, sizeof(v), socket);
    if (v>Max) {
      GeneralPanic();
    }
    v++;
    SocketWriteStruct((char*) &v, sizeof(v), socket);
  }
}

int main(int argc, char ** argv)
{
  KernelInit();
  SchedulerStartup();

  SocketInit(
      Buff1,
      BUFF_SIZE,
      Buff2,
      BUFF_SIZE,
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
