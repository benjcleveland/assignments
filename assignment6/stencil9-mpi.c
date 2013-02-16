#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"


//
// The logical *global* problem size -- N x N elements; each process
// will own a fraction of the whole.
//
#ifndef N
#define N 10
//#define N 1000
#endif

//
// We'll terminate when the difference between all elements in
// adjacent iterations is less than this value of epsilon.
//
#define epsilon .01
//#define epsilon .000001

// START OF PROVIDED ROUTINES (should not need to change)
// ------------------------------------------------------------------------------

//
// This routine is a really lame way of computing a square-ish grid of
// processes, favoring more columns than rows.  It returns the results
// via numRows and numCols.
//
void computeGridSize(int numProcs, int* numRows, int* numCols) {
  int guess = sqrt(numProcs);
  while (numProcs % guess != 0) {
    guess--;
  }
  *numRows = numProcs / guess;
  *numCols = guess;
}


//
// This routine calculates a given process's location within a virtual
// numRows x numCols grid, laying them out in row major order.
//
//
void computeGridPos(int me, int numRows, int numCols, int* myRow, int* myCol) {
  *myRow = me / numCols;
  *myCol = me % numCols;
}

// END OF PROVIDED ROUTINES (should not need to change)
// ------------------------------------------------------------------------------


int main(int argc, char* argv[]) {
  int numProcs, myProcID;
  int numRows, numCols;
  int myRow, myCol;

  //
  // Boilerplate MPI startup -- query # processes/images and my unique ID
  //
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myProcID);

  //
  // Arrange the numProcs processes into a virtual 2D grid (numRows x
  // numCols) and compute my logical position within it (myRow,
  // myCol).
  //
  computeGridSize(numProcs, &numRows, &numCols);
  computeGridPos(myProcID, numRows, numCols, &myRow, &myCol);

  //
  // Sanity check that we're up and running correctly.  Feel free to
  // disable this once you get things running.
  //
  printf("Process %d of %d checking in\n"
         "I am at (%d, %d) of %d x %d processes\n\n", myProcID, numProcs, 
         myRow, myCol, numRows, numCols);


  /* TODO (step 1): Using your block distribution (or a
     corrected/improved/evolved version of it) from assignment #1,
     compute the portion of the global N x N array that this task
     owns, using a block x block distribution */

  /* TODO (step 2): Allocate arrays corresponding to the local portion
     of data owned by this process -- in particular, don't allocate an
     O(N**2) array on each process, only the portion it owns.
     Allocate an extra row/column of data as a halo around the array
     to store global boundary conditions and/or overlap regions/ghost
     cells for caching neighboring processors' values, similar to what
     was shown for the 1D 3-point stencil in class, simply in 2D. */

  /* TODO (step 3): Initialize the arrays to zero. */

  /* TODO (step 4): Initialize the arrays to contain four +/-1.0
     values, as in assignment #5.  Note that you will need to do a
     global -> local index calculation to determine (a) which
     process(es) owns the points and (b) which array value the points
     correspond to. */

  /* TODO (step 5): Implement a routine to sequentially print out the
     distributed array to the console in a coordinated manner such
     that it appears as a global whole, as we logically think of it.
     In other words, the output of this routine should be identical to
     that of printArr() in assignment #5, in spite of the fact that
     the array is decomposed across a number of processes. Use
     Send/Recv calls to coordinate between the processes.  Use this
     routine to verify that your initialization is correct.
  */

  /* TODO (step 6): Implement the 9-point stencil using ISend/IRecv
     and Wait routines.  Use the non-blocking routines in order to get
     all the communication up and running in a safe manner.  While it
     is possible to compute on the innermost elements of the array
     before the communication completes, there is no reason to do so
     -- simply use the non-blocking calls as a means of getting a
     number of communications up and running without waiting for
     others to complete. */

  /* TODO (step 7): Verify that the stencil seems to be progressing
     correctly, as in assignment #5. */

  /* TODO (step 8): Use an MPI reduction to compute the termination of
     the routine, as in assignment #5. */

  /* TODO (step 9): Verify that the results of the computation (output
     array, number of iterations) are the same as assignment #5 for a
     few different problem sizes and numbers of processors; be sure to
     test a case in which there are interior processes (e.g., 9, 12,
     16, ... processes...) */

  MPI_Finalize();
  return 0;
}

