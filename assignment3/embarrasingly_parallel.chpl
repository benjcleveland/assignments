/*
 Ben Cleveland
 CSE P 524
 Assignment 2
*/

use Time;

// configuration variables
config const numElements = 100;
config const maxTasks = here.numCores;
config const maxValue = 16;
//config const distribution = false : bool;

class Distribute {
    var start$ : sync int;
    var size$ : sync int;
    var decay$ : int;
    var size : int;

    proc Distribute() {
        // initialize variables
        start$ = 1;
        size$ = (numElements/maxTasks)/2;
        decay$ = 0;
    }

    proc dynamicDistribution(items:range, taskId, numTasks, iteration) : range {
        /*
           var mystart = start$;
           var myend = mystart + size;

           if(myend > items.size) {
            myend = items.size;
           }
           start$ = myend + 1;

           var mysize = size$;
           writeln("size = ", size$);
           size$ = mysize/2;
           start$ = myend + 1;    
           end$ = size$ + myend;

         */

        var mysize = size$;
        // see if we need to update the size
        var mydecay = decay$;
        mydecay += 1;
        if(mydecay >= numTasks) {
            decay$ = 0;
            size$ = mysize/2;    
            //  writeln("size = ", size$);
        }
        else {
            //  writeln("size = ", size$);
            decay$ = mydecay;
            size$ = mysize; 
        }

        // determine the start
        var mystart = start$;
        // determine the end
        var myend = mystart + mysize;

        // make sure we don't go past the end of the array
        if(myend > items.size) {
            myend = items.size;
        }
        // set the next start
        start$ = myend + 1;

        return mystart..myend;
    }
}

// Compute all the factorials
proc factorialTask(items, itemRange, numTasks, myTask, distribution) {
    // compute the distribution
    var item_range : range(stridable=true);
    var factorial : int;
    var iteration = 1;
    var count = 0;

    const start_time = getCurrentTime();

    do {
        item_range = distribution.dynamicDistribution(itemRange, myTask, numTasks, iteration);
        // compute the factorial
        for i in item_range {
            factorial = 1;
            for j in 1..items[i] {
                factorial *= j;    
            }
            items[i] = factorial;
        }
        iteration += 1;
        count += item_range.size;
    } while (item_range.size > 0 );

    const elapsed_time = getCurrentTime() - start_time;
    return (iteration-1, elapsed_time,count);
}

// main entry point
proc main()
{
    var items: [1..numElements] int;
    var return_info: [0..#maxTasks] (int, real, int);

    var distribution = new Distribute();

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
    var total_count : sync int;
    total_count = 0;

    // create tasks
    coforall tid in 0..#maxTasks do
        return_info[tid] = factorialTask(items, 1..numElements, maxTasks, tid, distribution);


    // stop timer
    const elapsed_time = getCurrentTime() - start_time;

    for i in 0..#maxTasks {
        const (iterations, time, count) = return_info[i];
        writeln("Task: ", i, " Num Iterations: ", iterations, " Time: ", time, " Count: ", count);
        total_count += count;
    }

    // print the overall time
    writeln("Overall time: ", elapsed_time, " total count: ", total_count);
}
