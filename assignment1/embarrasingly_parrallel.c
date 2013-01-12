#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

typedef enum distribution
{
    BLOCK,
    CLYCLIC
}distribution_e;

typedef struct options
{
    distribution_e distribution;
    int num_tasks;
    int num_items;
    int *data;
}options_t;

typedef struct task
{   
    int task_id;
    pthread_t thread_id;
    options_t *ops;
}task_t;

/*
 * Determine which portion of the array this task has to deal with
 */
void computeMyBlockPart(int numItems, int numTasks, int myTaskID, int *myLo, int *myHi)
{
    // determine the number for each block
    int items_per_block = ceil(numItems/numTasks);

    // determine the high
    *myHi = myTaskID + 1 >= numTasks ? numItems : items_per_block * (myTaskID + 1); 

    // determine the low
    *myLo = items_per_block * myTaskID;
}

/*
 * Determine which portion of the array this task has to deal with
 */
void computeMyCyclicPart(int numItems, int numTasks, int myTaskID, int *myLo, int *myHi)
{

}

/*
 * Negate thread
 * 
 */
void* negateThread(void *arg)
{
    // cast arg
    task_t *task = (task_t *)arg;
    options_t *ops = task->ops;
    int i;
    int my_lo, my_hi;

    // determine compute part
    if(ops->distribution == BLOCK)
        computeMyBlockPart(ops->num_items, ops->num_tasks, task->task_id, &my_lo, &my_hi); 
    else 
        computeMyCyclicPart(ops->num_items, ops->num_tasks, task->task_id, &my_lo, &my_hi); 

    printf("thread %d lo %d, hi %d\n", task->task_id, my_lo, my_hi); 
    // do the computation
    for(i = my_lo; i < my_hi; ++i)
        ops->data[i] = -ops->data[i];

    // finish
    return NULL;
}

/*
 * Negate thread
 * 
 */
void* factorialThread(void *arg)
{
    // cast arg

    // determine compute part

    // do the computation

    // finish
    return NULL;
}

int main(void)
{
    struct timespec start_time;
    struct timespec end_time;
//    struct timespec overall_time;

    options_t ops;
    task_t  *tasks;

    int i;

    printf("Hello World!\n");

    // setup options
    ops.distribution = BLOCK;
    ops.num_items = 10000;
    ops.num_tasks = 4;

    //printf("asdf %lu %lu\n", sizeof(*ops.data), sizeof(ops.data));

    // allocate memory for the data
    ops.data = calloc(ops.num_items, sizeof(*ops.data));

    tasks = calloc(ops.num_tasks, sizeof(*tasks));

    // fill in data
    for(i = 0; i < ops.num_items; ++i)
        ops.data[i] = i;
    
    // start timer
    clock_gettime(CLOCK_MONOTONIC, &start_time); 

    // create tasks
    for(i = 0; i < ops.num_tasks; ++i)
    {
        tasks[i].task_id = i;
        tasks[i].ops = &ops;
        pthread_create(&tasks[i].thread_id, NULL, negateThread, &tasks[i]);
    }

    // join tasks - ignore the result for now
    for(i = 0; i < ops.num_tasks; ++i)
    {
        pthread_join(tasks[i].thread_id, NULL);
    }

    // stop timer
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // report results
    printf("Overall time: %i.%li\n", (int)(end_time.tv_sec - start_time.tv_sec), end_time.tv_nsec - start_time.tv_nsec);

    // cleanup
    if(ops.data != NULL)
        free(ops.data);
    if(tasks != NULL)
        free(tasks);

	return 0;
}
