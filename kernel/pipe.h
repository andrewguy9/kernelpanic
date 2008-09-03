#include"../utils/ringbuffer.h"
#include"semaphore.h"
#include"../utils/utils.h"

struct PIPE
{
	struct RING_BUFFER Ring;
	struct SEMAPHORE Mutex;
	struct SEMAPHORE ReaderLock;
	struct SEMAPHORE WriterLock;
};

void PipeInit( char * buff, COUNT size, struct PIPE * pipe );

COUNT PipeRead( char * buff, COUNT size, struct PIPE * pipe );
void PipeReadStruct( char * buff, COUNT size, struct PIPE * pipe );

COUNT PipeWrite( char * buff, COUNT size, struct PIPE * pipe );
void PipeWriteStruct( char * buff, COUNT size, struct PIPE * pipe );
