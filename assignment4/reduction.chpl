config const numTasks = 8;

//
// Each of these reductions will be called collectively by 'numTasks'
// tasks, each of which will pass in its taskID (mytid) from 0..numTasks-1.
//
// The routine should compute the sum reduction of the 'myval'
// arguments sent in by the tasks, returning the result on task 0
// (return values from other tasks will be ignored).  Though it's
// arguably poor form, it's expected that you'll use global variables
// to coordinate between the tasks calling into the routines.
//
// Note that, depending on your approach, one of the challenges in
// this problem will be to "reset" your global state so that
// subsequent calls to the routine will be correct.  If this bogs
// you down too much, separate your reductions into distinct files
// and test separately assuming they can only be called once (are
// not re-usable).
//

var computed_value$ : sync int = 0;
var done$ : sync int;
var task_count = 0;

proc reduction_a(mytid:int, myval:int): int {
  /* Implement part a, reduction using a single variable. */
  // need to wait for all the tasks to finish before the 0 returns... 
  var curr_value = computed_value$;
  curr_value = curr_value + myval;
  task_count += 1;
  if(task_count >= numTasks) {
    done$ = 1;
  }
  computed_value$ = curr_value;

  if(mytid == 0) {
    var read = done$;
    curr_value = computed_value$;
  }

  return curr_value;
}

var values$ : [0..numTasks] sync int;

proc reduction_b(mytid:int, myval:int): int {
  /* Implement part b, reduction with binary tree. Only needs to work
     for numTasks that are power of two */
  var next = mytid + 1;
  var iteration = 1;
  var local_val = myval;
  while(1) {
    // set value 
    //writeln(mytid, " ", next, " ", mytid & iteration);
    //writeln(mytid, " ", next, " ", mytid ^ iteration);
    // determine if we need to wait for a task
    if((mytid & iteration) != 0 || next >= numTasks ) {
        values$[mytid] = local_val;
      //  writeln("done ", mytid);
        break;
    }
    else {
        local_val += values$[next];
        //writeln("writing ", mytid);
        //values$[mytid] = local_val;      
    }
    next = next + iteration;
    iteration = iteration << 1;
  }
  //writeln("finished ", mytid);
  return local_val;
}


/*
proc reduction_c(mytid:int, myval:int): int {
  /* Implement part c, reduction with binary tree. Should work for any
     numTasks.  Paste solution to reduction_b here and modify */
}

proc reduction_d(mytid:int, myval:int, degree:int): int {
  /* NOTE: This part of the homework has been made completely optional
     and will not be graded.  Consider it a fun advanced step if you're
     enjoying the assignment; skip it if not. */

  /* Implement part d, reduction with degree-ary tree. Should work for any
     numTasks.  Paste solution to reduction_d here and modify */
}
*/
proc isPowerOf2( x: int ) {
   return (x & (x-1)) == 0;
}

proc test() {
  //
  // This is what you should expect to get as the output:
  //
  writeln("expected result: ", + reduce [i in 1..numTasks] i);

  //
  // Test reduction a
  //
  coforall tid in 0..#numTasks {
    //
    // for simplicity, reduce our task IDs: 1 + 2 + 3 + ... + numTasks
    //
    // the result only needs to be valid for task 0
    //
    const myResult = reduction_a(tid, tid+1);

    // copy result into shared variable
    if (tid == 0) then
      writeln("a: ", myResult);
  }

  //
  // Test reduction b
  //
  if (isPowerOf2(numTasks)) {
    coforall tid in 0..#numTasks {
      const myResult = reduction_b(tid, tid+1);

      // copy result into shared variable
      if (tid == 0) then
	    writeln("b: ", myResult);
    }
  } else {
    writeln("b: ", "does not accept non-power-of-2");
  }
/*
  //
  // Test reduction c
  //
  coforall tid in 0..#numTasks {
    const myResult = reduction_c(tid, tid+1);

    // copy result into shared variable
    if (tid == 0) then
      writeln("c: ", myResult);
  }

  //
  // Test reduction d
  //
  for degree in 2..5 {
    coforall tid in 0..#numTasks {
      const myResult = reduction_d(tid, tid+1, degree);

      // copy result into shared variable
      if (tid == 0) then
	writeln("d (",degree,"-ary): ", myResult);    
    }
  }
  */
}

test();
