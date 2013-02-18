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

/*
 * Determine which portion of the array this task has to deal with
 * using a block distribution
 *
 * Assumes that the number of tasks is less than the number if items
 *
 */
void computeMyBlockPart(int numItems, int numTasks, int myTaskID, int *myLo, int *myHi)
{
    // determine the number for each block
    int items_per_block = ceil((double)numItems/numTasks);

    // determine the high
    *myHi = myTaskID + 1 >= numTasks ? numItems : items_per_block * (myTaskID + 1); 
    if(*myHi > numItems)
        *myHi = numItems;
    // determine the low
    *myLo = (items_per_block * myTaskID) >= numItems ? numItems : items_per_block * myTaskID;
}


void printArray( int myProcID, int numProcs, int myRow, int myCol, int myNumRows, int myNumCols, int colStart, int colEnd,int numCols,  double **myArray) {
    int         rec_val;
    MPI_Status  status;

    FILE *fp;
    if(myRow != 0) {
        // wait for a signal
        //printf("waiting...! %d\n", myProcID);
        MPI_Recv(&rec_val, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        //printf("receiving %d!\n", myProcID);
    }

    for(int i = 1; i <= myNumRows; ++i) {
        if(myCol != 0 ) {
            //printf("waiting... for row! %d\n", myProcID);
            MPI_Recv(&rec_val, 1, MPI_INT, myProcID - 1, 1, MPI_COMM_WORLD, &status);
            //printf("receiving %d for row!\n", myProcID);
        }
        fp = fopen("tmp_file", "a");
        for(int j = 1; j <= myNumCols; ++j) {
            //printf("%5lf, ", myArray[i][j]);
            fprintf(fp, "%5lf ", myArray[i][j]);
            //fprintf(fp, "%d writing, ", myProcID);
        }

        if(colEnd != N) {
            fclose(fp);
            //printf("sending for row %d\n", myProcID);
            if(colStart == 0)
                rec_val = myProcID;
            MPI_Send(&rec_val, 1, MPI_INT, myProcID + 1, 1, MPI_COMM_WORLD);
            if(colStart == 0) 
                MPI_Recv(&rec_val, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            //printf("moving on %d\n", myProcID);
            // wait for the next row
        } else {
            fprintf(fp, "\n");
            fclose(fp);
            if(colStart != 0) {
                //printf("newline %d %d\n", myProcID, rec_val);
                MPI_Send(&myProcID, 1, MPI_INT, rec_val, 1, MPI_COMM_WORLD);
                //printf("newline sent %d\n", myProcID);
            }
        }
    }
    // if we are done writing all of our rows go to the next proc
    if(myProcID + 1 < numProcs && colEnd == N ) {
        for(int i = 0; i < numCols; ++i) {
            //printf("sending! %d %d\n", myProcID + 1 + i, myProcID);
            MPI_Send(&myProcID, 1, MPI_INT, myProcID + i + 1, 0, MPI_COMM_WORLD);
            //printf("sent!\n");
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if(myProcID == 0) {
        // display the file
        system("cat tmp_file; rm tmp_file");
    } 
}

int main(int argc, char* argv[]) {
  int numProcs, myProcID;
  int numRows, numCols;
  int myRow, myCol;
  
  int rowStart, rowEnd;
  int colStart, colEnd;
  int myNumRows, myNumCols;

  double **myArray;
  int rec_val;
  MPI_Status status;

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
  //printf("Process %d of %d checking in\n"
  //       "I am at (%d, %d) of %d x %d processes\n\n", myProcID, numProcs, 
  //       myRow, myCol, numRows, numCols);


  /* TODO (step 1): Using your block distribution (or a
     corrected/improved/evolved version of it) from assignment #1,
     compute the portion of the global N x N array that this task
     owns, using a block x block distribution */
  computeMyBlockPart(N, numRows, myRow, &rowStart, &rowEnd); 
  computeMyBlockPart(N, numCols, myCol, &colStart, &colEnd); 
  printf("Process %d of %d checking in\n"
         "I am at (%d, %d) of %d x %d processes\n"
         "rows (%d - %d)\n cols (%d - %d)\n", myProcID, numProcs, 
         myRow, myCol, numRows, numCols, rowStart, rowEnd, colStart, colEnd);

  myNumRows = rowEnd - rowStart;
  myNumCols = colEnd - colStart;

  /* TODO (step 2): Allocate arrays corresponding to the local portion
     of data owned by this process -- in particular, don't allocate an
     O(N**2) array on each process, only the portion it owns.
     Allocate an extra row/column of data as a halo around the array
     to store global boundary conditions and/or overlap regions/ghost
     cells for caching neighboring processors' values, similar to what
     was shown for the 1D 3-point stencil in class, simply in 2D. */
  
  // TODO -  is there a better way to do this?
  myArray = calloc(myNumRows+2, sizeof(*myArray));
  for(int i = 0; i < myNumRows+2; ++i)
      myArray[i] = calloc(myNumCols+2, sizeof(**myArray));

  /* TODO (step 3): Initialize the arrays to zero. */
  // done with the calloc

  /* TODO (step 4): Initialize the arrays to contain four +/-1.0
     values, as in assignment #5.  Note that you will need to do a
     global -> local index calculation to determine (a) which
     process(es) owns the points and (b) which array value the points
     correspond to. */
     // TODO - clean this up so there is not so much duplication
     int i = N/4;
     int j = N/4;
    /* int globalRowStart = (myRow != numRows -1) ? rowStart * myRow : rowStart;
     int globalRowEnd = (myRow != 0 && myRow != numRows - 1) ? rowEnd * myRow : rowEnd;
     int globalColStart = (myCol != numCols - 1) ? colStart * myCol : colStart;
     int globalColEnd = (myCol != 0 && myCol != numCols -1) ? colEnd * myCol : colEnd;
    */
    if( i >=  rowStart && i < rowEnd ) {
        if( j >= colStart && j < colEnd ) {      
            //printf("I have it! %d (%d, %d)\n", myProcID, i, j);
            myArray[i - rowStart+1][j - colStart+1] = 1;
        }
    }

    i = 3*N/4;
    j = 3*N/4;
    if( i >=  rowStart && i < rowEnd ) {
        if( j >= colStart && j < colEnd ) { 
            //printf("I have it! %d (%d, %d) rowStart (%d) colStart (%d)\n", myProcID, i, j, rowStart, colStart);
            myArray[i - rowStart+1][j - colStart+1] = 1;
        }
    }

    i = N/4;
    j = 3*N/4;
    if( i >=  rowStart && i < rowEnd ) {
        if( j >= colStart && j < colEnd ) {    
            //printf("I have it! %d (%d, %d)\n", myProcID, i, j);
            myArray[i - rowStart+1][j - colStart+1] = 1;
        }
    }

    i = 3*N/4;
    j = N/4;
    if( i >=  rowStart && i < rowEnd ) {
        if( j >= colStart && j < colEnd ) {      
            //printf("I have it! %d (%d, %d)\n", myProcID, i, j);
            myArray[i - rowStart+1][j - colStart+1] = 1;
        }
    }

/*

  printf("Process %d of %d checking in\n"
         "I am at (%d, %d) of %d x %d processes\n"
         "global rows (%d - %d)\n cols (%d - %d)\n", myProcID, numProcs, 
         myRow, myCol, numRows, numCols, globalRowStart, globalRowEnd, globalColStart, globalColEnd);
*/
  /* TODO (step 5): Implement a routine to sequentially print out the
     distributed array to the console in a coordinated manner such
     that it appears as a global whole, as we logically think of it.
     In other words, the output of this routine should be identical to
     that of printArr() in assignment #5, in spite of the fact that
     the array is decomposed across a number of processes. Use
     Send/Recv calls to coordinate between the processes.  Use this
     routine to verify that your initialization is correct.
  */
    printArray(myProcID, numProcs, myRow, myCol, myNumRows, myNumCols, colStart, colEnd, numCols, myArray );

 /* TODO (step 6): Implement the 9-point stencil using ISend/IRecv
     and Wait routines.  Use the non-blocking routines in order to get
     all the communication up and running in a safe manner.  While it
     is possible to compute on the innermost elements of the array
     before the communication completes, there is no reason to do so
     -- simply use the non-blocking calls as a means of getting a
     number of communications up and running without waiting for
     others to complete. */

    // receives
    MPI_Request requests[1000];
    MPI_Status  statuses[1000];
    int request_count = 0;

    // receive up
    MPI_Irecv(&myArray[0][1], myNumCols, MPI_DOUBLE, MPI_ANY_SOURCE, 0, 
            MPI_COMM_WORLD, &requests[request_count++]); 

    // receive below
    MPI_Irecv(&myArray[myNumRows][1], myNumCols, MPI_DOUBLE, MPI_ANY_SOURCE, 1, 
            MPI_COMM_WORLD, &requests[request_count++]); 

     // sends
     // send up
     //int dest = (myProcID == 0 ) ? numProcs - 1 : myProcID - 1;
     int dest = (myRow != 0) ? myCol + ((myRow - 1)%numRows)*numCols : myCol + (numRows-1)*numCols;
     //printf("up %d proc %d\n", dest, myProcID);
     MPI_Isend(&myArray[1][1], myNumCols, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD, &requests[request_count++]); 
     // send down
     //dest = (myProcID + 1) >= numProcs ? 0 : myProcID + 1;
     dest = myCol + ((myRow + 1)%numRows)*numCols;
     //printf("down %d proc%d \n", dest, myProcID);
     MPI_Isend(&myArray[myNumRows][1], myNumCols, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD, &requests[request_count++]);

    if(numCols > 1) {
        // send left
        dest = (myCol != 0) ? ((myCol - 1) % numCols) + ((myRow))*(numCols) : numCols-1 + myRow*numCols ;
        //printf("left %d myprocid %d dest = %d\n", (myCol), myProcID, dest);
        for(int i = 1; i <= myNumRows; ++i) {
            MPI_Isend(&myArray[i][1], 1, MPI_DOUBLE, dest, 2, MPI_COMM_WORLD, &requests[request_count++]);
            MPI_Irecv(&myArray[i][0], 1, MPI_DOUBLE, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &requests[request_count++]);
        }

        // send right
        dest = (myCol + 1) % numCols + ((myRow))*(numCols);
        //printf("right myprocid %d dest = %d\n", myProcID, dest);
        for(int i = 1; i <= myNumRows; ++i) {
            MPI_Isend(&myArray[i][myNumCols-1], 1, MPI_DOUBLE, dest, 3, MPI_COMM_WORLD, &requests[request_count++]);
            MPI_Irecv(&myArray[i][myNumCols], 1, MPI_DOUBLE, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &requests[request_count++]);
        }
    }

    // send corners
    //dest = (myRow != numRows - 1) ?(numCols + myProcID + 1) : myProcID + numCols+ 1 - numProcs;
    //dest = ((myCol + 1) % numCols)*numCols + ((myRow + 1) % numRows)*numRows;
    int useCol = numCols;
    if(myCol != 0)
        useCol = myCol;
    int useRow = numRows;
    if(myRow != 0)
        useRow = myRow;

    dest = (myCol + 1) % numCols + ((myRow + 1)%numRows)*(numCols);
    //printf("myprocid %d dest = %d\n", myProcID, dest);
    MPI_Isend(&myArray[myNumRows-1][myNumCols-1], 1, MPI_DOUBLE, dest, 4, MPI_COMM_WORLD, &requests[request_count++]); 
    MPI_Irecv(&myArray[0][0], 1, MPI_DOUBLE, MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, &requests[request_count++]); 

    dest = ((useCol - 1) % numCols) + ((useRow - 1)%numRows)*(numCols);
    //printf("myprocid %d dest = %d\n", myProcID, dest);
    MPI_Isend(&myArray[1][1], 1, MPI_DOUBLE, dest, 5, MPI_COMM_WORLD, &requests[request_count++]); 
    MPI_Irecv(&myArray[myNumRows][myNumCols], 1, MPI_DOUBLE, MPI_ANY_SOURCE, 5, MPI_COMM_WORLD, &requests[request_count++]); 

    dest = (useCol - 1) % numCols + ((myRow + 1)%numRows)*(numCols);
    //printf("myprocid %d dest = %d\n", myProcID, dest);
    MPI_Isend(&myArray[myNumRows-1][1], 1, MPI_DOUBLE, dest, 6, MPI_COMM_WORLD, &requests[request_count++]); 
    MPI_Irecv(&myArray[0][myNumCols], 1, MPI_DOUBLE, MPI_ANY_SOURCE, 6, MPI_COMM_WORLD, &requests[request_count++]); 

    dest = (myCol + 1) % numCols + ((useRow - 1)%numRows)*(numCols);
    //printf("myprocid %d dest = %d\n", myProcID, dest);
    MPI_Isend(&myArray[1][myNumCols-1], 1, MPI_DOUBLE, dest, 7, MPI_COMM_WORLD, &requests[request_count++]); 
    MPI_Irecv(&myArray[myNumRows][0], 1, MPI_DOUBLE, MPI_ANY_SOURCE, 7, MPI_COMM_WORLD, &requests[request_count++]); 

    // wait
    printf("requests %d\n", request_count);
    MPI_Waitall(request_count, requests, statuses);
  /* TODO (step 7): Verify that the stencil seems to be progressing
     correctly, as in assignment #5. */

  /* TODO (step 8): Use an MPI reduction to compute the termination of
     the routine, as in assignment #5. */

  /* TODO (step 9): Verify that the results of the computation (output
   *
     array, number of iterations) are the same as assignment #5 for a
     few different problem sizes and numbers of processors; be sure to
     test a case in which there are interior processes (e.g., 9, 12,
     16, ... processes...) */

  MPI_Finalize();
  return 0;
}


