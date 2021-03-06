/*
 * Ben Cleveland
 * Assignment 5
 * CSE P 524
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <accel.h>

//
// The logical problem size -- N x N elements
//
//#define N 10
#define N 1000

//
// We'll terminate when the difference between all elements in
// adjacent iterations is less than this value of epsilon.
//
//#define epsilon .01
#define epsilon .000001

//
// a utility routine for printing the inner N x N elements of
// a physical N+2 x N+2 array
//
void printArr(double A[N+2][N+2]) {
  int i, j;
  
  for (i=1; i<=N; i++) {
    for (j=1; j<=N; j++) {
      printf("%5lf ", A[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}


//
// This routine nitializes a logical N x N array stored as an 
// N+2 x N+2 array by storing +1.0 in the middle-ish of its 
// upper-left and lower-right quadrants; and -1.0 in the middle-ish 
// of the other two quadrants.
//
void initArr(double A[N+2][N+2]) {
  int i, j;

  //
  // Initialize the complete arrays to 0
  //
  for (i=0; i<N+2; i++) {
    for (j=0; j<N+2; j++) {
      A[i][j] = 0.0;
    }
  }

  //
  // Place a nonzero entry in the center of each quadrant.
  //
  A[N/4+1][N/4+1] = 1.0;
  A[3*N/4+1][3*N/4+1] = 1.0;
  A[N/4+1][3*N/4+1] = -1.0;
  A[3*N/4+1][N/4+1] = -1.0;
}

void timespec_diff(struct timespec start, struct timespec end, struct timespec *diff)
{
    if((end.tv_nsec - start.tv_nsec) < 0)
    {
        diff->tv_sec = end.tv_sec - start.tv_sec - 1;
        diff->tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        diff->tv_sec = end.tv_sec - start.tv_sec;
        diff->tv_nsec = end.tv_nsec - start.tv_nsec;
    }
}

//
// These are our two main work arrays -- we declare them to be N+2 x N+2
// even though our computation is on a logical N x N array in order to
// support boundary conditions and not have to worry about falling off
// the edges of the arrays
//
double X[N+2][N+2];
double Y[N+2][N+2];

int main() {
  int i, j;

  struct timespec start_time;
  struct timespec end_time;
  struct timespec diff_time;
    acc_init( acc_device_nvidia );

  initArr(X);
  
  double delta = 0.0;
  int numIters = 0;

  // start timer
  if(clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
      perror("Error from clock_gettime - getting start time!\n");

#pragma acc data copy(X,Y)
  do {
      numIters += 1;
      //
      // TODO: implement the stencil computation here
      //
      /*#pragma omp parallel for private(j) shared(X) shared(Y)*/
      #pragma acc kernels loop
      for( i = 1; i <=N; ++i) {
          for(j = 1; j <=N; ++j) {
              Y[i][j] = ((X[i][j]*.25) + (X[i+1][j] + X[i-1][j] + X[i][j+1] + X[i][j-1])*.125 + 
                      (X[i+1][j+1] + X[i-1][j+1] + X[i-1][j-1] + X[i+1][j-1])*.0625);
          }
      }
      // TODO: implement the computation of delta and get ready for the
      // next iteration here...
      //
      delta = 0;

      /*#pragma omp parallel for private(j) reduction(max:delta) shared(X) shared(Y)*/
      /*#pragma acc parallel loop reduction(max:delta)*/
      #pragma acc kernels loop 
      for(i = 1; i <=N; ++i) {
          for(j = 1; j <=N; ++j) { 
              double tmp_d = fabs(X[i][j] - Y[i][j]);
              delta = fmax(delta, tmp_d);
          }
      }
      // 1) check for termination here by computing delta -- the largest
      //    absolute difference between corresponding elements of X and Y
      //    (i.e., the biggest change due to the application of the stencil 
      //    for this iteration of the while loop)
      //
      // 2) copy Y back to X to set up for the next execution
      //
      /*#pragma omp parallel for private(j) shared(X) shared(Y)*/
      #pragma acc kernels loop 
      for(i = 1; i <= N; ++i) {
          for( j = 1; j <= N; ++j) {
              X[i][j] = Y[i][j];
          }
      }
  } while (delta > epsilon);

  // stop timer
  if(clock_gettime(CLOCK_MONOTONIC, &end_time) != 0)
      perror("Error from clock_gettime - getting end time!\n");

  timespec_diff(start_time, end_time, &diff_time);

  // report results
  printf("Overall time: %i.%09li\n delta %lf\n", (int)(diff_time.tv_sec), diff_time.tv_nsec, delta);

  //printArr(X);

  printf("Took %d iterations to converge\n", numIters);
}
