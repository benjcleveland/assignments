#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

typedef enum distribution
{
    BLOCK,
    CYCLIC
}distribution_e;

typedef enum operation
{
    FACTORIAL,
    NEGATE
}operation_e;

typedef enum input_deck
{
    RANDOM,
    VALUE_RAMP
}input_deck_e;

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
    int items_per_block = ceil((double)numItems/numTasks);

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

    *myLo = myTaskID;
    if(myTaskID < numItems % numTasks)
        *myHi = ((numItems/numTasks)) * numTasks + myTaskID + 1;
    else
        *myHi = ((numItems/numTasks) - 1) * numTasks + myTaskID + 1;
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
    int stride;
    int my_lo, my_hi;

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

    printf("thread %d lo %d, hi %d\n", task->task_id, my_lo, my_hi); 

    // do the computation
    for(i = my_lo; i < my_hi; i += stride)
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
                // TODO this will crash if an integer is not passed
                *num_tasks = atoi(optarg);
                break;
            case 'e': // number of elements
                *num_items = atoi(optarg);
                break;
            case 'm': // the max value in the input deck
                *max_value = atoi(optarg);
                break;
            default:
                printf("error\n");
                break;
        }
    }
    return 0;
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
    operation_e     oper;
    input_deck_e    input_deck;
    int             i;
    int             max_value = INT_MAX;
    int             items_per_block;

    printf("Hello World!\n");

    // defaults
    ops.num_tasks = 4;
    ops.num_items = 10000;
    ops.distribution = BLOCK;
    input_deck = RANDOM;

    // get options
    if( get_options(argc, argv, &oper, &ops.distribution, &input_deck, &ops.num_tasks, &ops.num_items, &max_value) == -1)
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

    tasks = calloc(ops.num_tasks, sizeof(*tasks));

    // seed the random number generator if needed
    if(input_deck == RANDOM)
        srand(time(NULL));
    
    if(input_deck == VALUE_RAMP)
        items_per_block = ceil((double)ops.num_items/max_value);

    // fill in data
    for(i = 0; i < ops.num_items; ++i)
    {
        if(input_deck == RANDOM)
            ops.data[i] = rand() % max_value;
        else // value ramp 
        {
            // determine the number for each block
            ops.data[i] = (i/items_per_block) + 1;
        }
            printf("data %d\n", ops.data[i]);
    }
    
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

    timespec_diff(start_time, end_time, &diff_time);

    // print the input values
    printf("Number of tasks: %d\n", ops.num_tasks);
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
    if(oper == FACTORIAL)
        printf("FACTORIAL\n");
    else if(oper == NEGATE)
        printf("NEGATE\n");

    // report results
    printf("%i %i Overall time: %i.%09li\n", (int)start_time.tv_sec, (int)end_time.tv_sec, (int)(diff_time.tv_sec), diff_time.tv_nsec);

    // cleanup
    if(ops.data != NULL)
        free(ops.data);
    if(tasks != NULL)
        free(tasks);

	return 0;
}
