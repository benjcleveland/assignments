#include <stdint.h>
#include <pthread.h>

typedef struct buffer_s {
    double          value;
    int             status;
    pthread_mutex_t lock;
    pthread_cond_t  full;
    pthread_cond_t  empty;
} Buffer_t;

typedef struct BoundedBuffer {
  
  /* FILL IN */
  uint64_t capacity;
  Buffer_t *buffer;
  
  pthread_mutex_t tail_m;
  pthread_mutex_t head_m;

  pthread_cond_t nonfull;
  pthread_cond_t nonempty;

  uint64_t head;
  uint64_t tail;

} BoundedBuffer;

// Initialize buffer to be empty and
// have the given capacity
void initBoundedBuffer( BoundedBuffer * b, uint64_t capacity );

// Insert the item into the buffer.
// Blocks until there is room in buffer. 
void produce( BoundedBuffer * buffer, double item );

// Delete one item in the buffer and return its value.       
// Blocks until the buffer is not empty.
double consume( BoundedBuffer * buffer );
