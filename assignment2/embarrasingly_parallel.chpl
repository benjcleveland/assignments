/*
 Ben Cleveland
 CSE P 524
 Assignment 2
*/

use Time;
use distributionCompute;

// configuration variables
config const numElements = 100;
config const maxTasks = here.numCores;
config const maxValue = 16;
config const distribution = false : bool;

// Compute all the factorials
proc factorialTask(items, itemRange, numTasks, myTask, distribution)
{
    // compute the distribution
    var item_range : range(stridable=true);
    var factorial : int;

    if(distribution == false) {
        item_range = computeMyBlockPart(itemRange, numTasks, myTask);
    }
    else {
        item_range = computeMyCyclicPart(itemRange, numTasks, myTask);
    }

    // compute the factorial
    for i in item_range {
        factorial = 1;
        for j in 1..items[i] {
            factorial *= j;    
        }
        items[i] = factorial;
    }
}

// main entry point
proc main()
{
    var items: [1..numElements] int;

    if(maxValue == 0) {
        writeln("Error invalid value for maxValue, exiting...");
        return;
    }

    if(maxTasks > numElements) {
        writeln("Error more tasks than elements, exiting...");
        return;
    }

    // fill the in the item array
    const step_size = ceil((numElements : real)/maxValue) : int;
    if( step_size == 0 ) {
        writeln("Error invalid step_size, exiting...");
        return;
    }

    for i in 1..numElements {
        items[i] = (((i-1)/step_size) + 1);
    }

    // start timer
    const start_time = getCurrentTime();

    // create tasks
    coforall tid in 0..#maxTasks do
        factorialTask(items, 1..numElements, maxTasks, tid, distribution);

    // stop timer
    const elapsed_time = getCurrentTime() - start_time;

    // print the overall time
    writeln("Overall time: ", elapsed_time);
}
