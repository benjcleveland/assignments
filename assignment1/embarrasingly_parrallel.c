/*
 * Ben Cleveland
 * CSE 524
 * Assignment 1 - Embarrasingly Parallel Performance Study
 *
 */

#include "work_distribution.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

// distribution types
typedef enum distribution
{
    BLOCK,
    CYCLIC
}distribution_e;

// operation types
typedef enum operation
{
    FACTORIAL,
    NEGATE
}operation_e;

// input decks
typedef enum input_deck
{
    RANDOM,
    VALUE_RAMP
}input_deck_e;

// options set for the current run
typedef struct options
{
    distribution_e  distribution;
    operation_e     operation;
    int             num_tasks;
    int             num_items;
    int64_t         *data;
}options_t;

// structure for given to each task
typedef struct task
{   
    int         task_id;
    pthread_t   thread_id;
    options_t   *ops;
}task_t;

/*
 * Negate thread
 * 
 */
void* computeThread(void *arg)
{
    // cast arg
    task_t      *task = (task_t *)arg;
    options_t   *ops = task->ops;
    int64_t         i, j;
    int         stride;
    int         my_lo, my_hi;

    // determine compute part
    if(ops->distribution == BLOCK)
    {
        computeMyBlockPart(ops->num_items, ops->num_tasks, task->task_id, &my_lo, &my_hi); 
        stride = 1;
    }
    else 
    {
        computeMyCyclicPart(ops->num_items, ops->num_tasks, task->task_id, &my_lo, &my_hi); 
        stride = ops->num_tasks;
    }

    //printf("thread %d lo %d, hi %d\n", task->task_id, my_lo, my_hi); 

    if(ops->operation == NEGATE)
    {
        // do the Negate computation
        for(i = my_lo; i < my_hi; i += stride)
            ops->data[i] = -ops->data[i];
    }
    else // compute factorial
    {
        int64_t factorial;
        for(i = my_lo; i < my_hi; i += stride)
        {   
            factorial = 1;
            for(j = 1; j <= ops->data[i]; ++j)
                factorial *= j;
            ops->data[i] = factorial;
        }
    }

    // finish
    return NULL;
}

/*
 * Handle command line options
 */
int get_options(int argc, char **argv, operation_e *oper, distribution_e *dist,
    input_deck_e *input_deck, int *num_tasks, int *num_items, int *max_value)
{
    int c;

    while((c = getopt(argc, argv, "o:i:d:n:e:m:")) != -1)
    {
        switch(c)
        {
            case 'o': // operation
                if(strcmp("factorial", optarg) == 0)
                    *oper = FACTORIAL;
                else if(strcmp("negate", optarg) == 0)
                    *oper = NEGATE;
                else
                    return -1;
            
                break;
            case 'd': // distribution
                if(strcmp("block", optarg) == 0)
                    *dist = BLOCK;
                else if(strcmp("cyclic", optarg) == 0)
                    *dist = CYCLIC;
                else
                    return -1;
                break;
            case 'i': // input deck
                if(strcmp("random", optarg) == 0)
                    *input_deck = RANDOM;
                else if(strcmp("valueramp", optarg) == 0)
                    *input_deck = VALUE_RAMP;
                else
                    return -1;
                break;
            case 'n': // number of tasks
                *num_tasks = strtol(optarg, NULL, 10);
                break;
            case 'e': // number of elements
                *num_items = strtol(optarg, NULL, 10);
                break;
            case 'm': // the max value in the input deck
                *max_value = strtol(optarg, NULL, 10);
                break;
            default:
                printf("error\n");
                break;
        }
    }
    return 0;
}

/*
 * Calculate the difference betweeen two timespec structures
 *
 * Stores the result in diff
 */
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

/*
 * Main
 */
int main(int argc, char **argv)
{
    struct timespec start_time;
    struct timespec end_time;
    struct timespec diff_time;

    options_t       ops;
    task_t          *tasks;
    input_deck_e    input_deck;
    int             i;
    int             max_value = INT_MAX;
    int             step_size = 1;

    // defaults
    ops.num_tasks = 4;
    ops.num_items = 10000;
    ops.distribution = BLOCK;
    ops.operation = NEGATE;
    input_deck = RANDOM;

    // get options
    if( get_options(argc, argv, &ops.operation, &ops.distribution, &input_deck, &ops.num_tasks, &ops.num_items, &max_value) == -1)
    {
        printf("Invalid agrument\n");
        return -1;
    }

    if(max_value > ops.num_items && input_deck == VALUE_RAMP)
    {
        printf("Error, max value is larger than number of items, illegal for value ramp input deck...\n");
        return -1;
    }

    //printf("asdf %lu %lu\n", sizeof(*ops.data), sizeof(ops.data));

    // allocate memory for the data
    ops.data = calloc(ops.num_items, sizeof(*ops.data));
    if(ops.data == NULL)
    {
        perror("Error allocating memory for data!\n");
        return -1;
    }

    tasks = calloc(ops.num_tasks, sizeof(*tasks));
    if(tasks == NULL)
    {
        perror("Error allocating memory for tasks!\n");
        return -1;
    }

    // seed the random number generator if needed
    if(input_deck == RANDOM)
        srand(time(NULL));
    
    // determine the 'step size' for the value ramp
    if(input_deck == VALUE_RAMP)
        step_size = ceil((double)ops.num_items/max_value);

    // fill in data
    for(i = 0; i < ops.num_items; ++i)
    {
        if(input_deck == RANDOM) // limit the random value to the max value passed in
            ops.data[i] = rand() % max_value;
        else // value ramp 
        {
            // determine the data value based on the step size
            ops.data[i] = (i/step_size) + 1;
        }
        //printf("data %d\n", ops.data[i]);
    }
    
    // start timer
    if(clock_gettime(CLOCK_MONOTONIC, &start_time) != 0)
        perror("Error from clock_gettime - getting start time!\n");

    // create tasks
    for(i = 0; i < ops.num_tasks; ++i)
    {
        tasks[i].task_id = i;
        tasks[i].ops = &ops;
        if(pthread_create(&tasks[i].thread_id, NULL, computeThread, &tasks[i]) != 0)
            perror("Error creating task!\n");
    }

    // join tasks - ignore the result for now
    for(i = 0; i < ops.num_tasks; ++i)
    {
        if( pthread_join(tasks[i].thread_id, NULL) != 0)
            perror("Error returned from pthread_join!\n");
    }

    // stop timer
    if(clock_gettime(CLOCK_MONOTONIC, &end_time) != 0)
        perror("Error from clock_gettime - getting end time!\n");

    timespec_diff(start_time, end_time, &diff_time);

    // print the input values
/*    printf("Number of tasks: %d\n", ops.num_tasks);
    printf("Number of items: %d\n", ops.num_items);
    printf("Distribution: ");
    if(ops.distribution == BLOCK)
        printf("BLOCK\n");
    else if(ops.distribution == CYCLIC)
        printf("CYCLIC\n");
    printf("Input Deck: ");
    if(input_deck == RANDOM)
        printf("RANDOM\n");
    else if(input_deck == VALUE_RAMP)
        printf("VALUE_RAMP\n");
    printf("Max Value: %d\n", max_value);
    printf("Operation: ");
    if(ops.operation == FACTORIAL)
        printf("FACTORIAL\n");
    else if(ops.operation == NEGATE)
        printf("NEGATE\n");
*/
    // report results
    printf("Overall time: %i.%09li\n", (int)(diff_time.tv_sec), diff_time.tv_nsec);

    for(i = 0; i < ops.num_items; ++i)
        printf("%li", ops.data[i]);
    // cleanup memory
    if(ops.data != NULL)
        free(ops.data);

    if(tasks != NULL)
        free(tasks);

	return 0;
}
