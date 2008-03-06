#ifndef PANIC_H
#define PANIC_H

enum ERROR_CODE 
{
	GENERIC_ERROR,
	//RESOURCE_SHARED_UNLOCK_WHEN_UNLOCKED,
	//RESOURCE_SHARED_UNLOCK_WRONG_MODE,
	//RESOURCE_INVALID_STATE,
	MUTEX_UNLOCK_WHILE_UNLOCKED,
	SCHEDULER_RESUME_THREAD_MUST_BE_CRIT,
	SCHEDULER_RESUME_THREAD_NOT_BLOCKED,	
	SCHEDULER_BLOCK_THREAD_MUST_BE_CRIT,
	SCHEDULER_START_CRITICAL_MUTEX_NOT_AQUIRED,
	SCHEDULER_END_CRITICAL_NOT_CRITICAL,
	SCHEDULE_MUST_BE_ATOMIC,
	TIMER_RUN_TIMERS_MUST_BE_ATOMIC,
	TIMER_REGISTER_MUST_BE_ATOMIC,
	HEAP_PROMOTE_TOP,
	HEAP_FIND_ELEMENT_OUT_OF_BOUNDS,
	HEAP_ADD_PARENT_FULL,
	HEAP_POP_TAIL_NULL,
	SCHEDULER_ACTIVE_THREAD_AWAKENED,
	HEAP_POP_TAIL_NOT_LEAF,
	LINKED_LIST_INSERT_NULL_NODE,
	LINKED_LIST_REMOVE_NULL_NODE,
	HAL_END_INTERRUPT_WRONG_LEVEL,
	SCHEDULER_FORCE_SWITCH_IS_CRITICAL,
	SCHEDULER_FORCE_SWITCH_IS_ATOMIC,
	TIMER_OVERFLOW_HAD_TIMERS,
	TIMER_REGISTER_TIMER_ALREADY_ACTIVE,
	SCHEDULER_SCHEDULE_NEXT_THREAD_NOT_NULL,
	SCHEDULER_CONTEXT_SWITCH_NOT_ATOMIC,
	SCHEDULER_GET_BLOCKING_CONTEXT_NOT_CRITICAL,
	SCHEDULER_GET_BLOCKING_CONTEXT_NOT_BLOCKED,
	PANIC2_VALIDATE_STATE_SEMAPHORE_OVERUSED,
	PANIC3_THREAD_MAIN_SLEPT_TOO_LITTLE,
	INTERRUPT_POST_HANDLER_REGISTER_NOT_ATOMIC,
	INTERRUPT_START_INTERRUPTS_INCONSISTENT,
	INTERRUPT_END_INTERRUPTS_INCONSISTENT,
	INTERRUPT_RUN_POST_HANDLERS_INCONSISTENT_STATE,
	INTERRUPT_IS_ATOMIC_WRONG_STATE,
	INTERRUPT_POST_HANDLER_REGISTER_ALREADY_ACTIVE,
	INTERRUPT_IS_POST_HANDLER_WRONG_STATE,
	INTERRUPT_ENABLE_OVER_ENABLED,
	SEMAHPORE_LOCK_COUNT_TOO_LARGE,
	SEMAPHORE_UNLOCK_COUNT_TOO_LARGE,
	//RESOURCE_SHARED_SHARED_WHILE_EXCLUSIVE,
	SLEEP_HANDLER_THREAD_AWAKE,
	RING_BUFFER_INIT_INVALID_SIZE,
	RING_BUFFER_INIT_SIZE_TOO_BIG,
	//RESOURCE_ESCALATE_NOT_SHARED,
	//RESOURCE_ESCALATE_NO_OWNERS,
	//RESOURCE_DEESCALATE_NOT_EXCLUSIVE,
	//RESOURCE_DEESCALATE_NUM_SHARED_NOT_ZERO,
	//RESOURCE_WAKE_THREADS_NOT_CRITICAL,
	IO_INPUT_BUFF_FULL,
	TEST2_PRODUCER_WRITE_WRONG_LENGTH,
	TEST2_CONSUMER_READ_WRONG_LENGTH,
	TEST2_CONSUMER_CHARACTER_MISMATCH,
	TEST3_READER_MISMATCH,
	RESOURCE_LOCK_EXCLUSIVE_INVALID_STATE,
	RESOURCE_LOCK_SHARED_INVALID_SATE,
	RESOURCE_UNLOCK_SHARED_WRONG_STATE,
	RESOURCE_UNLOCK_EXCLUSIVE_NUMSHARED_POSITIVE,
	RESOURCE_WAKE_THREADS_INVALID_CONTEXT,
	RESOURCE_UNLOCK_EXCLUSIVE_UNLOCK_UNEXPECTED,
	RESOURCE_ESCALATE_RESOURCE_NOT_SHARED,
	RESOURCE_DEESCALATE_RESOURCE_NOT_EXCLUSIVE,
	RESOURCE_DEESCALATE_RESOURCE_INCONSISTANT,
	RESOURCE_WAKE_THREADS_NUM_SHARED_NOT_ZERO,
	RESOURCE_WAKE_THREADS_NO_THREADS_TO_WAKE,
	TIMER_HANDLER_STACK_OVERFLOW,
	SCHEDULER_CONTEXT_SWITCH_STACK_OVERFLOW,
	RESOURCE_LOCK_SHARED_EXIT_WRONG_STATE,
	RESOURCE_LOCK_EXCLUSIVE_WRONG_EXIT,
	RESOURCE_ESCALATE_WRONG_EXIT_STATE,
	RESOURCE_DEESCALATE_BLOCK_STATE_BAD,
	RESOURCE_ESCALATE_NUM_SHARED_NOT_ZERO,
	LOCKING_BLOCK_WRONG_STATE,
	LOCKING_WAIT_WRONG_STATE,
	LOCKING_ACQUIRE_WRONG_STATE,
	LOCKING_ACQUIRE_INVALID_CONTEXT,
	LOCKING_IS_ACQUIRED_INVALID_CONTEXT,
	LOCKING_BLOCK_INVALID_CONTEXT,
	LOCKING_BLOCK_WRONG_CONTEXT,
	LOCKING_ACQUIRE_THREAD_IN_WRONG_STATE,
	LOCKING_ACQUIRE_CONTEXT_IN_WRONG_STATE,
	LOCKING_IS_ACQUIRED_CONTEXT_IN_WRONG_STATE,
	LOCKING_SWITCH_INVAID_STATE,
	TEST_3_WRITER_RESOURCE_NOT_EXCLUSIVE,
	TEST_3_READER_RESOURCE_NOT_SHARED,
	INTERRUPT_RUN_POST_HANDLERS_DIABABLED_HANDLER,
	TEST6_WORK_ITEM_DID_NOT_COMPLETE,
	WORKER_HANDLER_NOT_RUNNING,
	INTERRUPT_RUN_POST_HANDLERS_NOT_QUEUED
};

void GeneralPanic( enum ERROR_CODE error );
void KernelPanic( enum ERROR_CODE error );

#endif
