/*
 Ben Cleveland
 CSE P 524
 Assignment 2
*/

module distributionCompute {
    // calculate the block distribution
    proc computeMyBlockPart(items:range, numTasks:int, myTask:int) : range
    {
        // determine the number for each block
        const items_per_block = ceil((items.length : real)/numTasks) : int;
    
        // compute the low
        const lo = items_per_block * myTask + 1;
    
        // compute the high
        var hi : int;
        if((myTask + 1) >= numTasks) {
            hi = items.high;
        } else {
            hi = items_per_block * (myTask + 1);
        }
    
        return lo..hi;
    }
    
    // calculate the cyclic distribution
    proc computeMyCyclicPart(items:range, numTasks:int, 
        myTask:int) : range(stridable=true) {
    
        return items.low..items.high by numTasks align (myTask + 1);
    
    }
}
