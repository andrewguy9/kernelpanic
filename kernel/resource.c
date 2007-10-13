#include"resource.h"
#include"scheduler.h"
#include"../utils/utils.h"

void ResourceInit( struct RESOURCE * lock )
{
	lock->State = RESOURCE_SHARED;
	LinkedListInit( &lock->ExclusiveQueue );
	LinkedListInit( &lock->SharedQueue );
	lock->NumShared = 0;
}

void ResourceLockShared( struct RESOURCE * lock )
{
	SchedulerStartCritical();
	switch( lock->State )
	{
		case RESOURCE_SHARED:
			lock->NumShared++;
			SchedulerEndCritical();
			return;

		case RESOURCE_ESCALATING:
		case RESOURCE_EXCLUSIVE:
			SchedulerBlockThread();
			LinkedListEnqueue( 
					(struct LINKED_LIST_LINK *) SchedulerGetActiveThread(),
					& lock->SharedQueue);
			SchedulerForceSwitch();
			return;
	}
}

BOOL ResourceLockSharedNonBlocking( struct RESOURCE * lock )
{
	SchedulerStartCritical();
	if( lock->State == RESOURCE_SHARED )
	{
		lock->NumShared++;
		SchedulerEndCritical();
		return TRUE;
	}
	else
	{
		SchedulerEndCritical();
		return FALSE;
	}
}

void ResourceLockExclusive( struct RESOURCE * lock )
{
	SchedulerStartCritical();
	switch( lock->State )
	{
		case RESOURCE_SHARED:
			if( lock->NumShared == 0)
			{//The lock is completely free, switch to exclusive
				lock->State = RESOURCE_EXCLUSIVE;
				SchedulerEndCritical();
				return;
			}
			else
			{
				//The lock is held by shared threads, 
				//we must wait until they finish.
				lock->State = RESOURCE_ESCALATING;
				SchedulerBlockThread();
				LinkedListEnqueue( 
						(struct LINKED_LIST_LINK *) SchedulerGetActiveThread(),
						& lock->ExclusiveQueue );
				SchedulerForceSwitch();
				return;
			}

		case RESOURCE_ESCALATING:
		case RESOURCE_EXCLUSIVE:
			//the lock is already escalating, add self to exclusive queue.
			SchedulerBlockThread();
			LinkedListEnqueue( 
					(struct LINKED_LIST_LINK*) SchedulerGetActiveThread(),
					& lock->ExclusiveQueue );
			SchedulerForceSwitch();
			return;
	}
}

BOOL ResourceLockExclusiveNonBlocking( struct RESOURCE * lock )
{
	SchedulerStartCritical();
	if( lock->State == RESOURCE_SHARED && lock->NumShared == 0 )
	{
		lock->State = RESOURCE_EXCLUSIVE;
		SchedulerEndCritical();
		return TRUE;
	}
	
	//Could not aquire lock immediatly.
	SchedulerEndCritical();
	return FALSE;
}

void ResourceUnlockShared( struct RESOURCE * lock )
{
	struct THREAD * sleeper;

	SchedulerStartCritical();
	ASSERT( lock->State != RESOURCE_EXCLUSIVE, 
			"Someone unlocked shared when lock was held exclusively." );
	ASSERT( lock->NumShared > 0, 
			"Someone unlocked shared when no one had it locked shared");

	lock->NumShared--;
	if( lock->State == RESOURCE_ESCALATING && lock->NumShared == 0)
	{
		//all of the threads we are waiting on have finished,
		//switch to exclusive.
		lock->State = RESOURCE_EXCLUSIVE;
		//wake the first exclusive thread
		sleeper = (struct THREAD *) LinkedListPop( & lock->ExclusiveQueue );
		SchedulerResumeThread( sleeper );
	}
	SchedulerEndCritical();
}

void ResourceUnlockExclusive( struct RESOURCE * lock )
{
	struct THREAD * sleeper;
	SchedulerStartCritical();
	ASSERT( lock->State == RESOURCE_EXCLUSIVE,
		   	"Someone tried to unlock exclusive while the lock was not exclusive.");

	if( ! LinkedListIsEmpty( & lock->ExclusiveQueue ) )
	{//there are more exclusive threads waiting, pass off execution to them.
		sleeper = (struct THREAD *) LinkedListPop( & lock->ExclusiveQueue );
		SchedulerResumeThread( sleeper );
	}
	else
	{//there are NO more threads waiting for exclusive. We should wake every shared thread
		while( !LinkedListIsEmpty( & lock->SharedQueue ) )
		{
			sleeper = (struct THREAD *) LinkedListPop( & lock->SharedQueue );
			SchedulerResumeThread( sleeper );
		}
	}
	SchedulerEndCritical();
}
