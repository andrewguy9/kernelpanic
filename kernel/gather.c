#include"gather.h"

void GatherInit( struct GATHER * gather, COUNT count )
{
	gather->Needed = count;
}

void GatherSync( struct GATHER * gather, struct LOCKING_CONTEXT * context )
{
	LockingStart();

	//Mark that new thread has arrived
	gather->Present++;
	//see if everyone is here
	if( gather->Needed == gather->Present )
	{
		//everyone is here
		gather->Present = 0;
		//tell new guy that he can continue
		Acquire( context );
		//Tell everyone waiting they can continue.
		while( ! LinkedListIsEmpty( & gather->List ) )
		{
			struct LOCKING_CONTEXT curContext;
			curContext = BASE_OBJECT(
					LinkedListPop( &lock->WaitingThreads ),
					struct LOCKING_CONTEXT,
					Link);
			Acquire( curContext );
		}
		//We are done, everyone is awake
		LockingEnd();
	}
	else
	{
		//We are not the last man, 
		//so we need to block.
		LockingBlock( NULL, context );

		//We need to store ourself away for later retreval.
		LinkedListEnqueue( & context->Link, & gather->List );

		//We may need to switch threads.	
		LockingSwitch( context );
	}
}

