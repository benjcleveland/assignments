/*
  Ben Cleveland
  CSE P 524
  Assignment 3
*/

use Time;

config const capacity = 4;
config const P = 2;
config const C = 2;
config const N = 1000;

const TERM = -1.0;

class BoundedBuffer {
    /* FILL IN FIELDS */
    var head$ : sync int;
    var tail$ : sync int;
    var buff$ : [0..#capacity] sync real;
    var cap = capacity : int;

    proc BoundedBuffer() {
        /* IMPLEMENT */
        cap = capacity;
        head$ = 0;
        tail$ = 0;
    }

    proc produce( item:real ) : void {
        /* IMPLEMENT */
        var myhead = head$;
        head$ = (myhead + 1) % cap ;
        buff$[myhead] = item;
    }

    proc consume( ) : real {
        /* IMPLEMENT */
        var mytail = tail$;
        tail$ =  (mytail + 1) % cap;
        return buff$[mytail];
    }
}


/* Test the buffer */

// consumer task procedure
proc consumer(b: BoundedBuffer, id: int) {
    // keep consuming until it gets a TERM element
    var count = 0;
    do {
        const data = b.consume();
        count += 1;
    } while (data!=TERM);

    return count-1;
}

// producer task procedure
proc producer(b: BoundedBuffer, id: int) {
    // produce my strided share of N elements
    var count = 0;
    for i in 0..#N by P align id {
        //writeln(id, " producing ", i);
        b.produce(i);
        count += 1;
    }

    return count;
}

// produce an element that indicates that the consumer should exit
proc signalExit(b: BoundedBuffer) {
    b.produce(TERM);
}

var P_count: [0..#P] int;
var C_count: [0..#C] int;
var theBuffer = new BoundedBuffer();
const start_time = getCurrentTime();
cobegin {
    sync {
        begin {
            // spawn P producers
            coforall prodID in 0..#P do
                P_count[prodID] = producer(theBuffer, prodID);

            // when producers are all done, produce C exit signals
            for consID in 0..#C do
                signalExit(theBuffer);
        }
    }
   
    // spawn C consumers 
    coforall consID in 0..#C do
        C_count[consID] = consumer(theBuffer, consID);
}    

const elapsed_time = getCurrentTime() - start_time;

writeln("P_count=", P_count);
writeln("C_count=", C_count);

// print totals
var total_c = 0;
for i in 0..#C do
    total_c += C_count[i];
writeln("total consumer=", total_c);

total_c = 0;
for i in 0..#P do
    total_c += P_count[i];
writeln("total producer=", total_c);

writeln("Total time= ", elapsed_time);
