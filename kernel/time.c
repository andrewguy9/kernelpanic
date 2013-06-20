#include"time.h"
#include"hal.h"

/*
 * Time Unit Description:
 * The timer unit keeps time for the kernel. The current time can be queried
 * by calling TimeGet().
 */

TIME TimeGet()
{
        return HalGetTime();
}

