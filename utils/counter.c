#include"counter.h"

void CounterInit(struct COUNTER * c)
{
  c->val = 0;
}

void CounterContextInit(struct COUNTER * c, struct COUNTER_CONTEXT * ctx)
{
  ctx->val = c->val;
}

COUNT CounterCheck(struct COUNTER * c, struct COUNTER_CONTEXT * ctx)
{
  COUNT old = ctx->val;
  ctx->val = c->val;
  return ctx->val - old;
}

void CounterAdd(struct COUNTER * c, COUNT diff)
{
  c->val += diff;
}

