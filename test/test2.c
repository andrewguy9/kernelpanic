#include"../kernel/startup.h"
#include"../kernel/scheduler.h"
#include"../kernel/semaphore.h"
#include"../kernel/timer.h"
#include"../kernel/hal.h"
#include"../kernel/sleep.h"
#include"../kernel/panic.h"

//
//Tests the Producer consumer model for the socket system. 
//

char String1[] = "Notice that incrementing the variables must not be interrupted, and the P operation must not be interrupted after s is found to be non-zero. This can be done using a special instruction such as test-and-set (if the architecture's instruction set supports it), or (on uniprocessor systems) ignoring interrupts in order to prevent other processes from becoming active.The canonical names P and V come from the initials of Dutch words. V stands for verhoog, or \"increase\". Several explanations have been given for P (including passeer for \"pass\", probeer \"try\", and pakken \"grab\"), but in fact Dijkstra wrote that he intended P to stand for the made-up portmanteau word prolaag,[1] short for probeer te verlagen, or \try-and-decrease\" [2][3] (A less ambiguous, and more accurate, English translation would be \"try-to-decrease\".) This confusion stems from the unfortunate characteristic of the Dutch language that the words for increase and decrease both begin with the letter V, and the words spelled out in full would be impossibly confusing for non–Dutch-speakers. The value of a semaphore is the number of units of the resource which are free. (If there is only one resource, a \"binary semaphore\" with values 0 or 1 is used.) The P operation busy-waits (or maybe sleeps) until a resource is available, whereupon it immediately claims one. V is the inverse; it simply makes a resource available again after the process has finished using it. Init is only used to initialize the semaphore before any requests are made. The P and V operations must be atomic, which means that no process may ever be preempted in the middle of one of those operations to run another operation on the same semaphore. In the programming language ALGOL 68, in the Linux kernel,[1] and in some English textbooks, the P and V operations are called, respectively, down and up. In software engineering practice, they are often called wait and signal, or acquire and release, or pend and post. To avoid busy-waiting, a semaphore may have an associated queue of processes (usually a FIFO). If a process performs a P operation on a semaphore which has the value zero, the process is added to the semaphore's queue. When another process increments the semaphore by performing a V operation, and there are processes on the queue, one of them is removed from the queue and resumes execution."

char String2[] = "In computer science, the producer-consumer problem (also known as the bounded-buffer problem) is a classical example of a multi-process synchronization problem. The problem describes two processes, the producer and the consumer, who share a common, fixed-size buffer. The producer's job is to generate a piece of data, put it into the buffer and start again. At the same time the consumer is consuming the data (i.e. removing it from the buffer) one piece at a time. The problem is to make sure that the producer won't try to add data into the buffer if it's full and that the consumer won't try to remove data from an empty buffer. The solution for the producer is to go to sleep if the buffer is full. The next time the consumer removes an item from the buffer, it wakes up the producer who starts to fill the buffer again. In the same way the consumer goes to sleep if it finds the buffer to be empty. The next time the producer puts data into the buffer, it wakes up the sleeping consumer. The solution can be reached by means of inter-process communication, typically using semaphores. An inadequate solution could result in a deadlock where both processes are waiting to be awakened. The problem can also be generalized to have multiple producers and consumers."

//Allocation for buffers.
#define RING_SIZE
char buff1[RING_SIZE];
char buff2[RING_SIZE];

struct PIPE Pipe1;
struct PIPE Pipe2;

//Allocation for workers. 
#define STACK_SIZE 300
char ProducerStack1[STACK_SIZE];
char ProducerStack2[STACK_SIZE];
char ConsumerStack1[STACK_SIZE];
char ConsumerStack2[STACK_SIZE];
char ConsumerStack3[STACK_SIZE];
struct THREAD Producer1;
struct THREAD Producer2;
struct THREAD Consumer1;
struct THREAD Consumer2;
struct THREAD Consumer3;

//Functions for test.
void ProducerTest(char * string )
{
	INDEX index = 0;
	INDEX start=0;
	while( string[index] != '\0')
	{
		if( string[index] )
	}
}

void ProducerMain1()
{
	ProducerTest( String1 );
}

void ProducerMain2()
{
	ProducerTest( String2 );
}

void ConsumerMain()
{
}

//main
int main()
{
	//Initialize Pipes.
	PipeInit( buff1, RING_SIZE, &Pipe1 );
	PipeInit( buff2, RING_SIZE, &Pipe2 );

	//Initialize Threads
	SchedulerCreateThread(
			&Producer1,
			1,
			ProducerStack1,
			STACK_SIZE,
			ProducerMain1);
	SchedulerCreateThread(
			&Producer2,
			1,
			ProducerStack2,
			STACK_SIZE,
			ProducerMain2);
	SchedulerCreateThread(
			&Consumer1,
			1,
			ConsumerStack1,
			STACK_SIZE,
			ConsumerMain);
	SchedulerCreateThread(
			&Consumer2,
			1,
			ConsumerStack2,
			STACK_SIZE,
			ConsumerMain);
	SchedulerCreateThread(
			&Consumer3,
			1,
			ConsumerStack3,
			STACK_SIZE,
			ConsumerMain);
	//Kick off the kernel.
	KernelStart();
}
