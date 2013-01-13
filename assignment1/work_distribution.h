#ifndef _WORK_DISTRIBUTION_H
#define _WORK_DISTRIBUTION_H

void computeMyBlockPart(int numItems, int numTasks, int myTaskID, int *myLo, int *myHi);
void computeMyCyclicPart(int numItems, int numTasks, int myTaskID, int *myLo, int *myHi);
#endif
