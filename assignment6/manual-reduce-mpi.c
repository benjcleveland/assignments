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

  //printf("Process %d of %d checking in\n", myProcID, numProcs);

  /* TODO: Implement a reduction to compute the sum of all the proc
     IDs using MPI send/recv routines rather than the built-in MPI
     reduction.  This should essentially be the distributed memory
     rewrite of your previous homework (HW4, Q5c), using messages to
     synchronize between the tasks (processes) rather than
     synchronization variables. */

  int iteration = 1;
  int local_val = myProcID;
  int rec_val;
  MPI_Status status;

  while(1) {
    // determine if we need to wait for a task
    if((myProcID & iteration) != 0 ) {
        // send the value
        // we need to figure out who we are going to send to
        //printf("%d sending to %d %d\n", myProcID, (myProcID ^ iteration), iteration);
        MPI_Send(&local_val, 1, MPI_INT, myProcID - iteration, 0, MPI_COMM_WORLD);
        //printf("sent!\n");
        break;
    }
    else {
        int source = myProcID ^ iteration;
        if( source < numProcs ) {
            // add the other tasks value to our value
            // receive the value
            //printf("%d waiting for %d\n", myProcID, source, (myProcID ^ iteration));
              // TODO - handle errors
            MPI_Recv(&rec_val, 1, MPI_INT, source, 0, MPI_COMM_WORLD, &status); 
            //printf("Recived! %d\n", rec_val);
            local_val += rec_val;
        }
        else if(myProcID == 0)
                break;
    }
    iteration = iteration << 1;
  }

  if(myProcID == 0) {
    int expected = 0;
    for(int i = 0; i < numProcs; ++i)
        expected += i;
      printf("The final value is %d, expected %d\n", local_val, expected);
  }

  MPI_Finalize();
  return 0;
}
