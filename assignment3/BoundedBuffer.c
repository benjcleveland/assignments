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
  // create mutex lock
    pthread_mutex_init(&b->mutex, NULL);   
    pthread_mutex_init(&b->tail_m, NULL);   
    pthread_mutex_init(&b->head_m, NULL);   
    // create condition variables

    pthread_cond_init(&b->nonfull, NULL);
    pthread_cond_init(&b->nonempty, NULL);
    b->head = 0;
    b->tail = 0;
}

void produce( BoundedBuffer * b, double item ) {
  /* IMPLEMENT */

  pthread_mutex_lock(&b->head_m);
  pthread_mutex_lock(&b->mutex);
  while(((b->head + 1) % b->capacity) == b->tail) 
      pthread_cond_wait(&b->nonfull, &b->mutex);

  pthread_mutex_unlock(&b->mutex);

  // add the item to the buffer
  b->buffer[b->head] = item;
  b->head = (b->head + 1) % b->capacity;

  pthread_mutex_lock(&b->mutex);
  pthread_cond_signal(&b->nonempty);
  pthread_mutex_unlock(&b->mutex);

  pthread_mutex_unlock(&b->head_m);
}

double consume( BoundedBuffer * b ) {
  /* IMPLEMENT */
    double ret;
    pthread_mutex_lock(&b->tail_m);
    pthread_mutex_lock(&b->mutex);
    while(b->head == b->tail) 
        pthread_cond_wait(&b->nonempty, &b->mutex);
    pthread_mutex_unlock(&b->mutex);

    ret = b->buffer[b->tail];
    b->tail = (b->tail +1) %b->capacity;

    pthread_mutex_lock(&b->mutex);
    pthread_cond_signal(&b->nonfull);
    pthread_mutex_unlock(&b->mutex);

    pthread_mutex_unlock(&b->tail_m);
    return ret;
}


// producer thread procedure
void * producer( void * arg ) {
  uint64_t id = (uint64_t) arg;

  // produces 0..#N by P align id
  uint64_t count = 0;
  for ( uint64_t j=id; j<N; j+=P ) {
      printf("%d producing %li\n", id, j);
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
    printf("consuming %d\n", data);
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

