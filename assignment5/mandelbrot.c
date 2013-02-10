#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>

//
// The maximum number of colors to permit in the PPM file
//
#define maxColor 15


//
// The dimensions of the resulting image file -- feel free to make it bigger
//
#ifndef ROWS
//#define ROWS 201
#define ROWS 8001
#endif
#ifndef COLS
#define COLS ROWS
#endif

//
// The maximum number of steps the Mandelbrot iteration should run before
// giving up if it doesn't converge
//
#ifndef MAXSTEPS
#define MAXSTEPS 50
#endif

// ================================================================
// Start HELPER ROUTINES: Should not require modification

//
// A helper routine to plot the Mandelbrot set.  'NumSteps' is the
// image array indicating the number of steps each pixel required
// to converge.  maxUsed is the largest value contained within that
// array.
//
static void plot(int NumSteps[ROWS][COLS], int maxUsed) {
  const char* filename = "mandelbrot.ppm";
  FILE* outfile = fopen(filename, "w");
  int i,j;

  fprintf(outfile, "P3\n");
  fprintf(outfile, "%d %d\n", ROWS, COLS);
  fprintf(outfile, "%d\n", maxUsed);
  for (i=0; i<ROWS; i++) {
    for (j=0; j<COLS; j++) {
      fprintf(outfile, "%d 0 0 ", ((maxUsed*NumSteps[i][j])/MAXSTEPS));
    }
    fprintf(outfile, "\n");
  }
  fclose(outfile);
}

//
// Map an image coordinate to a point in the complex plane.
// Image coordinates are (row, col), with row 0 at the top.
//
int mapImg2CPlane(int row, int col, double* re, double* im) {
  const double rmin = -1.5;
  const double rmax = 0.5;
  const double imin = -1.0;
  const double imax = 1.0;

  *re = (rmax - rmin) * (double)col / COLS + rmin;
  *im = (imin - imax) * (double)row / ROWS + imax;
}

//
// Given a coordinate in the space (0..#rows, 0..#cols), compute the
// number of steps required to converge
//
int coordToNumSteps(int x, int y) {
  double Cre, Cim;
  mapImg2CPlane(x,y,&Cre,&Cim);

  double Zre = 0.0, Zim = 0.0;
  int i;
  //  printf("For (%d,%d), c is (%lf,%lf)\n", x,y,Cre,Cim);
  for (i=1; i<=MAXSTEPS; i++) {
    //    printf("%f %f\n", Zre, Zim);
    double newZre = (Zre*Zre - Zim*Zim) + Cre;
    double newZim = (Zre*Zim + Zim*Zre) + Cim;
    Zre = newZre;
    Zim = newZim;
    if (sqrt(Zre*Zre + Zim*Zim) > 2.0) {
      return i;
    }
  }
  return 0;
}


// =========================================================================
// END HELPER ROUTINES


//
// This will form the image for the Mandelbrot set; each
// value corresponds to the number of steps required to
// converge, or 0 if it didn't converge.
//
int NumSteps[ROWS][COLS];

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


int main() {
  int i,j;
  int max_step = 0;

   struct timespec start_time;
  struct timespec end_time;
  struct timespec diff_time;

    // start timer
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
        perror("Error from clock_gettime - getting start time!\n");

  //
  // This is a completely bogus computation to fill the array with a
  // color ramp as a placeholder until you do the real computation
  //
  //for (i=0; i<ROWS; i++) {
  //  for (j=0; j<COLS; j++) {
  //    NumSteps[i][j] = (i+j)*MAXSTEPS / (ROWS+COLS);
  //  }
  //}

  //
  // TODO: Replace the loop above with the real mandelbrot computation
  // by passing coordinates of the NumSteps array into coordToNumSteps()
  // and storing the resulting number of steps into the corresponding
  // array element
  //
  #pragma omp parallel for private(j) //schedule(dynamic)
  for (i=0; i<ROWS; i++) {
    for (j=0; j<COLS; j++) {
      NumSteps[i][j] = coordToNumSteps(i,j);
    }
  }

  //
  // TODO: Replace the MAXSTEPS argument below with the largest value
  // in the NumSteps array (in case no pixels required the full MAXSTEPS
  // iterations, and to get practice with OpenMP reductions.
  //
  #pragma omp parallel for private(j) reduction(max:max_step)
  for (i=0; i<ROWS; i++) {
    for (j=0; j<COLS; j++) {
      max_step = fmax(max_step,NumSteps[i][j]);
    }
  }
  
  // stop timer
    if(clock_gettime(CLOCK_MONOTONIC, &end_time) != 0)
        perror("Error from clock_gettime - getting end time!\n");

    timespec_diff(start_time, end_time, &diff_time);

    // report results
    printf("Overall time: %i.%09li\n", (int)(diff_time.tv_sec), diff_time.tv_nsec);

  //plot(NumSteps, MAXSTEPS);
  plot(NumSteps, max_step);
}
