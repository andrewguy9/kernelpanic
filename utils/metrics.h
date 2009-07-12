#ifndef METRICS_H
#define METRICS_H

#include"utils.h"
#include"bham.h"

/*
 * Unit which will measures a number of events against a threshold.
 * The user will provide a number of trials and a number maximum number
 * of events. Then the user calls MeterTick a series of times, one for each
 * trail. If the trail was an event they specifiy that. Then the user can 
 * call MeterCheck to see if the number of events is highter than their 
 * specified maximum.
 *
 * The design goal of this unit is to use a small amount of memory. So rather
 * than using a trail ring buffer, we will use an event counter and decay rate.
 * If the event occurances outpace the decay we will signal.
 */

struct METER {

	struct BHAM Accumulator;
	COUNT CurrentEvents;
	COUNT Sensitivity;
};

void MeterPercentage( COUNT events, COUNT trials, COUNT sensitivity, struct METER * meter );
void MeterTick(BOOL eventState, struct METER * meter);
BOOL MeterCheck(struct METER * meter);

#endif //METRICS_H
