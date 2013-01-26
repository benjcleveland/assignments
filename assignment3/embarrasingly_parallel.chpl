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
config const distribution = false : bool;


var start$ : sync int;
start$ = 1;
var size$ : int;
//config const size = (numElements/maxTasks)/2;
size$ = (numElements/maxTasks)/(3/2);
var end$ : sync int;
var decay$ : sync int;
decay$ = 0;
end$ = size$;

proc dynamicDistribution(items:range, taskId, numTasks, iteration) : range {
    /*
    var mystart = start$;
    var myend = end$;
    if(myend > items.size) {
        myend = items.size;
    }

    var mysize = size$;
    writeln("size = ", size$);
    size$ = mysize/2;
    start$ = myend + 1;    
    end$ = size$ + myend;

    */

    // determine the start
    var mystart = start$;
    // determine the end
    var myend = mystart + size$;

    // make sure we don't go past the end of the array
    if(myend > items.size) {
        myend = items.size;
    }
    
    // set the next start
    start$ = myend + 1;
    
    // increment decay
    decay$ += 1;
    
    var mydecay = decay$;
    if(mydecay >= numTasks) {
        size$ = size$/(3/2);    
      //  writeln("size = ", size$);
        decay$ = 0;
    }
    else {
      //  writeln("size = ", size$);
        decay$ = mydecay;
    }

    return mystart..myend;
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
        item_range = dynamicDistribution(itemRange, myTask, numTasks, iteration);
        //writeln(myTask, " ", item_range);
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
    return (iteration, elapsed_time,count);
}

// main entry point
proc main()
{
    var items: [1..numElements] int;
    var iterations: [0..#maxTasks] int;
    var task_time: [0..#maxTasks] real;

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
    coforall tid in 0..#maxTasks {
        var count = 0;
        (iterations[tid], task_time[tid], count) = factorialTask(items, 1..numElements, maxTasks, tid, distribution);
        total_count += count;
    }

    // stop timer
    const elapsed_time = getCurrentTime() - start_time;

    for i in 0..#maxTasks {
        writeln(i, " Num Iterations: ", iterations[i], " Time: ", task_time[i]);
    }

    // print the overall time
    writeln("Overall time: ", elapsed_time, " total count: ", total_count);
}
