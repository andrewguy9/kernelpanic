#include"metrics.h"

//TODO REMOVE THIS UNIT
void MeterPercentage( COUNT events, COUNT trials, COUNT sensitivity, struct METER * meter )
{
	ASSERT( meter != NULL );
	
	//Set up decay.
	BHamInit( trials, events, &meter->Accumulator );
	//Set up thresholds.
	meter->CurrentEvents = 0;
	meter->Sensitivity = sensitivity;
}

void MeterTick(BOOL eventState, struct METER * meter)
{
	BOOL decay;

	ASSERT( meter != NULL );

	if( eventState ) 
		meter->CurrentEvents++;
	
	decay = BHamTick( &meter->Accumulator );
	if( decay )
	{
		if(meter->CurrentEvents > 0)
			meter->CurrentEvents--;
	}
}

/*
 * Returns TRUE if we are within the desired event rate
 */
BOOL MeterCheck(struct METER * meter)
{
	ASSERT( meter != NULL );
	return meter->CurrentEvents <= meter->Sensitivity;
}
