#include <stdio.h>
#include <stdlib.h>
#include "BoundedBuffer.h"

/* config */ uint64_t P = 2;
/* config */ uint64_t C = 2;
/* config */ uint64_t N = 1000;
/* config */ uint64_t capacity = 4;

BoundedBuffer the_buffer;

const double TERM = -1.0f;

void initBoundedBuffer( BoundedBuffer * b, uint64_t capacity ) {
    /* IMPLEMENT */
    // allocate memory for the buffer
    b->buffer = calloc(capacity, sizeof(*b->buffer));

    b->capacity = capacity;
    // create mutex locks
    pthread_mutex_init(&b->tail_m, NULL);   
    pthread_mutex_init(&b->head_m, NULL);   

    // initialize each buffer element
    for(int i = 0; i < b->capacity; ++i) {
        pthread_mutex_init(&(b->buffer[i].lock), NULL);
        pthread_cond_init(&(b->buffer[i].empty), NULL);
        pthread_cond_init(&(b->buffer[i].full), NULL);
        b->buffer[i].status = 0; // empty
    }

    b->head = 0;
    b->tail = 0;
}

void produce( BoundedBuffer * b, double item ) {
    /* IMPLEMENT */
    int myhead;

    // get the head index
    pthread_mutex_lock(&b->head_m);
    myhead = b->head;
    b->head = (myhead + 1) % b->capacity;
    pthread_mutex_unlock(&b->head_m);

    pthread_mutex_lock(&b->buffer[myhead].lock);
    while(b->buffer[myhead].status == 1) // element is full, wait for empty
        pthread_cond_wait(&b->buffer[myhead].empty, &b->buffer[myhead].lock);

    b->buffer[myhead].value = item;
    b->buffer[myhead].status = 1; // full

    // wake up anyone waiting to read
    pthread_cond_signal(&b->buffer[myhead].full);
    pthread_mutex_unlock(&b->buffer[myhead].lock);

}

double consume( BoundedBuffer * b ) {
    /* IMPLEMENT */
    double  ret;
    int     mytail;

    pthread_mutex_lock(&b->tail_m);
    mytail = b->tail;
    b->tail = (mytail + 1) %b->capacity;
    pthread_mutex_unlock(&b->tail_m);

    pthread_mutex_lock(&b->buffer[mytail].lock);
    while(b->buffer[mytail].status == 0) // element is empty wait for the next one
        pthread_cond_wait(&b->buffer[mytail].full, &b->buffer[mytail].lock);

    ret = b->buffer[mytail].value;
    b->buffer[mytail].status = 0; // empty

    // wake up anyone waiting to write
    pthread_cond_signal(&b->buffer[mytail].empty);
    pthread_mutex_unlock(&b->buffer[mytail].lock);

    return ret;
}


// producer thread procedure
void * producer( void * arg ) {
  uint64_t id = (uint64_t) arg;

  // produces 0..#N by P align id
  uint64_t count = 0;
  for ( uint64_t j=id; j<N; j+=P ) {
      //printf("%d producing %li\n", id, j);
    produce( &the_buffer, j );
    count++;
  }

  return (void*)count;
}

// consumer thread procedure
void * consumer( void * ignore ) {
  // consumes items until it eats a TERM element
  uint64_t count = 0;
  double data;
  do {
    data = consume( &the_buffer );
    count++;
    //printf("consuming %f\n", data);
  } while (data != TERM);

  return (void*)(count-1);
}


int main(int argc, char ** argv) {
  if (argc != 5) {
    printf("usage: %s <capacity> <N> <P> <C>\n", argv[0]);
    exit(1);
  }

  // command line parse
  capacity = atoi(argv[1]);
  N = atoi(argv[2]);
  P = atoi(argv[3]);
  C = atoi(argv[4]);
   
  initBoundedBuffer( &the_buffer, capacity );

  // create P producers
  pthread_t P_thread_ids[P];
  for ( int64_t i=0; i<P; i++ ) {
    pthread_create( &P_thread_ids[i], NULL, producer, (void*) i);
  }
  
  // create C consumers
  pthread_t C_thread_ids[C];
  for ( int64_t i=0; i<C; i++ ) {
    pthread_create( &C_thread_ids[i], NULL, consumer, NULL);
  }

  // join on producers
  uint64_t P_count[P];
  for ( int64_t i=0; i<P; i++ ) {
    void * status;
    pthread_join( P_thread_ids[i], &status );
    P_count[i] = (uint64_t) status;
  }

  // create C signal exits to kill consumers
  for ( int64_t i=0; i<C; i++ ) {
    produce(&the_buffer, TERM);
  }

  // join on consumers
  uint64_t C_count[C];
  for ( int64_t i=0; i<C; i++ ) {
    void * status;
    pthread_join( C_thread_ids[i], &status );
    C_count[i] = (uint64_t) status;
  }


  // print out p/c counts
  printf("== TOTALS ==\n");
  printf("P_Count=");
  for ( int64_t i=0; i<P; i++ ) {
    printf(" %ld", P_count[i]);
  }
  printf("\n");
  printf("C_Count=");
  int64_t total_count = 0;
  for ( int64_t i=0; i<C; i++ ) {
    printf(" %ld", C_count[i]);
    total_count += C_count[i];
  }
  printf("\n");
  printf("Total C_Count=%li\n", total_count); 
}

