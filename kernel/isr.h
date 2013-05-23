#ifndef ISR_H
#define ISR_H

#include"utils/utils.h"
#include"hal.h"

//
//Unit Management
//

void IsrStartup();

//
//Register new ISR
//

void IsrRegisterHandler( ISR_HANDLER handler, void * which, enum IRQ_LEVEL level );

//
//Handle Atomic Sections
//

void IsrDisable(enum IRQ_LEVEL level);

void IsrEnable(enum IRQ_LEVEL level);

void IsrIncrement(enum IRQ_LEVEL level);

void IsrDecrement(enum IRQ_LEVEL level);

void IsrDefer( enum IRQ_LEVEL level, BOOL enable );

//
//Functions for Sanity Checking
//

#ifdef DEBUG
BOOL IsrIsAtomic(enum IRQ_LEVEL level);
BOOL IsrIsEdge(enum IRQ_LEVEL level);
#endif//DEBUG

#endif

