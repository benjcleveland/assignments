#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char* argv[]) {
  int numProcs, myProcID;

  //
  // Boilerplate MPI startup -- query # processes/images and my unique ID
  //
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myProcID);

  printf("Process %d of %d checking in\n", myProcID, numProcs);

  /* TODO: Implement a reduction to compute the sum of all the proc
     IDs using MPI send/recv routines rather than the built-in MPI
     reduction.  This should essentially be the distributed memory
     rewrite of your previous homework (HW4, Q5c), using messages to
     synchronize between the tasks (processes) rather than
     synchronization variables. */

  MPI_Finalize();
  return 0;
}
