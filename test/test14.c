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

struct PIPE Pipe1;
struct PIPE Pipe2;
struct SOCKET Socket1;
struct SOCKET Socket2;

#include<stdio.h>
THREAD_MAIN CountMain;
void CountMain(void * context) {
  struct SOCKET * socket = /*(struct socket *)*/ context;
  int v;
  printf("%p started\n", context);
  if (socket == &Socket1) {
    printf("%p kickstarted\n", context);
    v = 0;
    SocketWriteStruct((char*) &v, sizeof(v), socket);
  }

  while (1) {
    printf("%p reading\n", context);
    SocketReadStruct((char*) &v, sizeof(v), socket);
    if (v>Max) {
      return;
    }
    v++;
    printf("%p writing %d\n", context, v);
    SocketWriteStruct((char*) &v, sizeof(v), socket);
  }
}

int main(int argc, char ** argv)
{
  KernelInit();
  SchedulerStartup();

  PipeInit( Buff1, BUFF_SIZE, &Pipe1 );
  PipeInit( Buff2, BUFF_SIZE, &Pipe1 );
  SocketInit( &Pipe1, &Pipe2, &Socket1 );
  SocketInit( &Pipe2, &Pipe1, &Socket2 );

  SchedulerCreateThread(
      &CountThread1,
      100,
      CountStack1,
      STACK_SIZE,
      CountMain,
      &Socket1,
      TRUE);

  SchedulerCreateThread(
      &CountThread2,
      100,
      CountStack2,
      STACK_SIZE,
      CountMain,
      &Socket2,
      TRUE);

  KernelStart();

  return 0;
}
