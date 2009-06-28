#include<stdio.h>

#include"metrics.h"

int main()
{
	COUNT events;
	COUNT trails;
	COUNT sensitivity;
	int choice;

	struct METER meter;

	printf("How many events\n");
	scanf("%d", &events);

	printf("How many trials\n");
	scanf("%d", &trails);

	printf("What sensitivity\n");
	scanf("%d", &sensitivity);

	MeterPercentage( events, trails, sensitivity, &meter);

	printf("Entering trial loop\n");
	while(1) 
	{
		printf("1: Event, 2: Failure, 3: Exit\n");
		scanf("%d", &choice);

		switch(choice)
		{
			case 1:
				MeterTick(TRUE, &meter);
				break;
			case 2:
				MeterTick(FALSE, &meter);
				break;
			case 3:
				printf("goodbye\n");
				exit(0);
				break;
			default:
				printf("Invalid choice\n");
				break;
		}

		if(MeterCheck(&meter))
		{
			printf("VALID: ");
		}
		else
		{
			printf("THRESHOLD EXCEEDED :");
		}
		printf("Current Errors %d, Error Max %d\n", meter.CurrentEvents, meter.Sensitivity);

	} //trail loop.
	return 0;
}
