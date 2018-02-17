#include"pipe.h"

/*
 * Unit Description:
 * Provides the concept of a Pipe.
 * Pipes are a single direction byte based communication mechanism.
 * A thread can call PipeWrite to put information into the pipe
 * which can be consumed by another thread through PipeRead.
 *
 * Calls to pipe functions can cause threads to block.
 */

/*
 * Initializes a pipe
 */
void PipeInit( char * buff, COUNT size, struct PIPE * pipe, PIPE_READ * pr, PIPE_WRITE * pw )
{
        SemaphoreInit( & pipe->Mutex, 1 );
        SemaphoreInit( & pipe->EmptyLock, 0 ); //Buffer starts empty, block reads.
        SemaphoreInit( & pipe->FullLock, 1 );//Buffer starts empty, allow writes.
        SemaphoreInit( & pipe->ReadLock, 1 );
        SemaphoreInit( & pipe->WriteLock, 1 );
        RingBufferInit( buff, size, & pipe->Ring );
        * pr = pipe;
        * pw = pipe;
}

COUNT PipeReadInner( char * buff, COUNT size, PIPE_READ pipe ) {
        BOOL wasFull;
        BOOL dataLeft;
        COUNT read;

        //Acquire EmptyLock - No readers can progress until there is data.
        SemaphoreDown( & pipe->EmptyLock, NULL );

        //Acqure mutex lock - No one can do any io until
        //we leave the buffer.
        SemaphoreDown( & pipe->Mutex, NULL );

        //Check and see if the buffer is full.
        //If it is, then the FullLock should
        //have been leaked, and writers should be blocking.
        wasFull = RingBufferIsFull( & pipe->Ring );
        ASSERT( wasFull ? (pipe->FullLock.Count == 0) : TRUE );

        //Perform the read.
        read = RingBufferRead( buff, size, & pipe->Ring );
        // Ring is protected by a read lock which should prevent zero length reads.
        ASSERT( read > 0 );

        //See if the ring buffer is empty.
        //If it is then we need to leak the reader lock.
        dataLeft = !RingBufferIsEmpty( & pipe->Ring );

        //Release mutex lock - We are out of the ring, so
        //let other IO go if its already passed.
        SemaphoreUp( & pipe->Mutex );

        //If the ring was full while we have exclusive access,
        //and we  freed up some space then we should release
        //the writer lock so writers can go.
        if( wasFull && read > 0 ) {
                SemaphoreUp( & pipe->FullLock );
        }

        //If there is data left in the buffer, release the
        //empty lock so that other readers can go.
        //If there is no data in the buffer, we cant let
        //readers progress, so we leak the lock.
        if( dataLeft ) {
                SemaphoreUp( & pipe->EmptyLock );
        }
        return read;
}
/*
 * Reads data from a pipe.
 *
 * Arguments:
 * buff - destination buffer
 * size - maximum length that will be read.
 * pipe - pipe we will read from
 *
 * Returns
 * The length of data we read.
 *
 * The size of the read data will be min( data in pipe, size)
 */
COUNT PipeRead( char * buff, COUNT size, PIPE_READ pipe )
{
        COUNT read;
        SemaphoreDown( & pipe->ReadLock, NULL );
        read = PipeReadInner(buff, size, pipe);
        SemaphoreUp( & pipe->ReadLock);
        return read;
}

void PipeReadStruct( char * buff, COUNT size, PIPE_READ pipe )
{
        COUNT read = 0;
        SemaphoreDown( & pipe->ReadLock, NULL );
        while (read < size) {
                read += PipeReadInner(buff+read, size-read, pipe);
        }
        SemaphoreUp( & pipe->ReadLock);
        ASSERT(read == size);
}

COUNT PipeWriteInner( char * buff, COUNT size, PIPE_WRITE pipe ) {
        BOOL wasEmpty;
        BOOL spaceLeft;
        COUNT write;

        //Acquire FullLock - No writers can progress until we are done.
        SemaphoreDown( & pipe->FullLock, NULL );

        //Acqure mutex lock - No one can do any io until
        //we leave the buffer.
        SemaphoreDown( & pipe->Mutex, NULL );

        //Check and see if the buffer is empty.
        //If it is, then the EmptyLock should
        //have been leaked, and readers should be blocking.
        wasEmpty = RingBufferIsEmpty( & pipe->Ring );
        ASSERT( wasEmpty ? (pipe->EmptyLock.Count == 0) : TRUE );

        //Perform the write.
        write = RingBufferWrite( buff, size, & pipe->Ring );
        // Ring is protected by a write lock which should prevent zero length writes.
        ASSERT( write > 0 );

        //See if the ring buffer is empty.
        //If it is then we need to leak the reader lock.
        spaceLeft = !RingBufferIsFull( & pipe->Ring );

        //Release mutex lock - We are out of the ring, so
        //let other IO go if its already passed.
        SemaphoreUp( & pipe->Mutex );

        //If the ring was empty while we have exclusive access,
        //and we wrote some data, then we should release
        //the EmptyLock so readers can go.
        if( wasEmpty && write > 0 ) {
                SemaphoreUp( & pipe->EmptyLock );
        }

        //If there is space left in the buffer, release the
        //FullLock so that other writers can go.
        //If there is no space in the buffer, we cant let
        //writers progress, so we leak the lock.
        if( spaceLeft ) {
                SemaphoreUp( & pipe->FullLock );
        }
        return write;
}

/*
 * Writes data to a pipe.
 *
 * Arguments
 * buff - buffer we will read from
 * size - the maximum distance we will read from.
 * pipe - the pipe we will copy data to.
 *
 * Returns
 * The length of data we read from buff.
 *
 * The size of the written data will be min( space left in pipe, size)
 */
COUNT PipeWrite( char * buff, COUNT size, PIPE_WRITE pipe )
{
        COUNT write;
        SemaphoreDown( & pipe->WriteLock, NULL );
        write = PipeWriteInner(buff, size, pipe);
        SemaphoreUp( & pipe->WriteLock);
        return write;
}

void PipeWriteStruct( char * buff, COUNT size, PIPE_WRITE pipe )
{
        COUNT write = 0;
        SemaphoreDown( & pipe->WriteLock, NULL );
        while (write < size) {
                write += PipeWriteInner(buff+write, size-write, pipe);
        }
        SemaphoreUp( & pipe->WriteLock);
        ASSERT(write == size);
}

