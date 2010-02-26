#include<stdio.h>
#include<stdlib.h>

#include"metrics.h"

//TODO remove this unit.
void CheckState( struct METER * meter)
{
	if(MeterCheck(meter))
	{
		printf("VALID: ");
	}
	else
	{
		printf("THRESHOLD EXCEEDED :");
	}
	printf("Current Errors %ld, Error Max %ld\n", meter->CurrentEvents, meter->Sensitivity);
}

int main()
{
	COUNT cur;

	COUNT events;
	COUNT trails;
	COUNT sensitivity;

	INDEX choice;

	int randSeed;
	COUNT randEvents;
	COUNT randTrials;
	COUNT repeat;
	BOOL result;
	COUNT invalid;
	COUNT trailEvents;

	struct METER meter;

	srand(randSeed);

	printf("How many events\n");
	scanf("%ld", &events);

	printf("How many trials\n");
	scanf("%ld", &trails);

	printf("What sensitivity\n");
	scanf("%ld", &sensitivity);

	MeterPercentage( events, trails, sensitivity, &meter);

	printf("Entering trial loop\n");
	while(1) 
	{
		printf("1: Event, 2: Failure, 3 rand attempt, 4: Exit\n");
		scanf("%ld", &choice);

		switch(choice)
		{
			case 1:
				MeterTick(TRUE, &meter);
				CheckState(&meter);
				break;
			case 2:
				MeterTick(FALSE, &meter);
				CheckState(&meter);
				break;
			case 3:
				printf("Fraction of events: a/b\n");
				printf("a:");
				scanf("%ld", &randEvents);
				printf("b:");
				scanf("%ld", &randTrials);
				printf("num attempts:");
				scanf("%ld", &repeat);

				trailEvents = 0;
				invalid = 0;
				for(cur = 0;cur < repeat; cur++)
				{
					result = (rand() % randTrials) < randEvents;
					if(result) 
					{
						printf("E ");
						trailEvents++;
					}
					else
					{
						printf("  ");
					}
					MeterTick( result, &meter );
					CheckState(&meter);
					if(!MeterCheck(&meter))
						invalid++;
				}
				printf("%ld/%ld were events, %ld invalid states\n", trailEvents, repeat, invalid);
				break;
			case 4:
				printf("goodbye\n");
				exit(0);
				break;
			default:
				printf("Invalid choice\n");
				break;
		}

	} //trail loop.
	return 0;
}
