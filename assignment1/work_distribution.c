#include "work_distribution.h"
#include <math.h>

/*
 * Determine which portion of the array this task has to deal with
 */
void computeMyBlockPart(int numItems, int numTasks, int myTaskID, int *myLo, int *myHi)
{
    // determine the number for each block
    int items_per_block = ceil((double)numItems/numTasks);

    // determine the high
    *myHi = myTaskID + 1 >= numTasks ? numItems : items_per_block * (myTaskID + 1); 

    // determine the low
    *myLo = items_per_block * myTaskID;
}

/*
 * Determine which portion of the array this task has to deal with
 */
void computeMyCyclicPart(int numItems, int numTasks, int myTaskID, int *myLo, int *myHi)
{

    *myLo = myTaskID;
    if(myTaskID < numItems % numTasks)
        *myHi = ((numItems/numTasks)) * numTasks + myTaskID + 1;
    else
        *myHi = ((numItems/numTasks) - 1) * numTasks + myTaskID + 1;
}
