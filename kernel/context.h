#ifndef CONTEXT_H
#define CONTEXT_H

#include"utils/utils.h"
#include"hal.h"

typedef void THREAD_MAIN(void * arg ); //TODO CHANGE NAME.

// AKA STACK
struct CONTEXT {
  struct MACHINE_CONTEXT MachineState;
  STACK_INIT_ROUTINE  * Main; // Not needed for ctx switch, but needed for bootstrapping.
  void * MainArg; // Not needed for ctx switch, but needed for bootstrapping.

#ifdef DEBUG
        struct COUNTER TimesRun;
        struct COUNTER TimesSwitched;
        volatile TIME LastRanTime;
        volatile TIME LastSelectedTime;
        //Pointers to the top and bottom of the stack. Used to detect stack overflow.
        void * High;
        void * Low;
#endif
};

void ContextInit( struct CONTEXT * context, char * pointer, COUNT Size, STACK_INIT_ROUTINE Foo, void * Arg );
void ContextSwitch(struct CONTEXT * oldStack, struct CONTEXT * newStack);

#endif
