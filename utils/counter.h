#ifndef COUNTER_H
#define COUNTER_H

#include"utils.h"

struct COUNTER
{
  COUNT val;
};

struct COUNTER_CONTEXT 
{
  COUNT val;
};

void CounterInit(struct COUNTER * c);
void CounterContextInit(struct COUNTER * c, struct COUNTER_CONTEXT * ctx);
COUNT CounterCheck(struct COUNTER * c, struct COUNTER_CONTEXT * ctx);
void CounterAdd(struct COUNTER * c, COUNT diff);

#endif
