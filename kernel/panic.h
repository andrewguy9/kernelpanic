#ifndef PANIC_H
#define PANIC_H

enum ERROR_CODE 
{
	GENERIC_ERROR,
	RESOURCE_SHARED_UNLOCK_WHEN_UNLOCKED,
	RESOURCE_SHARED_UNLOCK_WRONG_MODE,
	RESOURCE_INVALID_STATE,
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
	HEAP_POP_TAIL_NOT_LEAF,
	LINKED_LIST_INSERT_NULL_NODE,
	LINKED_LIST_REMOVE_NULL_NODE,
	HAL_END_INTERRUPT_WRONG_LEVEL,
	SCHEDULER_FORCE_SWITCH_IS_CRITICAL,
	SCHEDULER_FORCE_SWITCH_IS_ATOMIC,
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
	RESOURCE_SHARED_SHARED_WHILE_EXCLUSIVE,
	SLEEP_HANDLER_THREAD_AWAKE,
	RING_BUFFER_INIT_INVALID_SIZE,
	RING_BUFFER_INIT_SIZE_TOO_BIG,
	RESOURCE_ESCALATE_NOT_SHARED,
	RESOURCE_ESCALATE_NO_OWNERS,
	RESOURCE_DEESCALATE_NOT_EXCLUSIVE,
	RESOURCE_DEESCALATE_NUM_SHARED_NOT_ZERO,
	RESOURCE_WAKE_THREADS_NOT_CRITICAL,
	IO_INPUT_BUFF_FULL
};

void GeneralPanic( enum ERROR_CODE error );
void KernelPanic( enum ERROR_CODE error );

#endif
