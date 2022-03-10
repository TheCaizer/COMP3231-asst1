/* This file will contain your solution. Modify it as you wish. */
#include <types.h>
#include <lib.h>
#include <synch.h>
#include "producerconsumer.h"

/* The bounded buffer uses an extra free slot to distinguish between
   empty and full. See
   http://www.mathcs.emory.edu/~cheung/Courses/171/Syllabus/8-List/array-queue2.html
   for details if needed. 
*/

#define BUFFER_SIZE (BUFFER_ITEMS + 1)

/* Declare any variables you need here to keep track of and
   synchronise the bounded buffer. The declaration of a buffer is
   shown below. It is an array of pointers to data_items.
*/


data_item_t * item_buffer[BUFFER_SIZE];

volatile int write_head, read_tail;
struct lock *buffer_lock;
struct semaphore *full;
struct semaphore *empty;



/* The following functions test if the buffer is full or empty. They
   are obviously not synchronised in any way */
/* using semaphore therefore no need for this function
static bool is_full() {
        return (write_head + 1) % BUFFER_SIZE == read_tail;
}

static bool is_empty() {
        return write_head == read_tail;
}
*/
/* consumer_receive() is called by a consumer to request more data. It
   should block on a sync primitive if no data is available in your
   buffer. It should not busy wait! Any concurrency issues involving
   shared state should also be handled. 
*/

data_item_t * consumer_receive(void)
{
        data_item_t * item;

    //    using semaphore therefore no need for this function
    //    while(is_empty()) {
    //            /* busy wait */
    //    }
        // wait on the full semaphore
        P(full);
        // accessing buffer so lock it for mutal exclusion use
        lock_acquire(buffer_lock);
        item = item_buffer[read_tail];
        read_tail = (read_tail + 1) % BUFFER_SIZE;
        // finished using the buffer therefore release the lock
        lock_release(buffer_lock);
        // signal the empty semaphore
        V(empty);

        return item;
}

/* producer_send() is called by a producer to store data in the
   bounded buffer.  It should block on a sync primitive if no space is
   available in the buffer. It should not busy wait! Any concurrency
   issues involving shared state should also be handled.
*/

void producer_send(data_item_t *item)
{
    // using semaphore therefore no need for this function
    //    while(is_full()) {
    //            /* busy wait */
    //   }
        // wait on the empty semaphore
        P(empty);
        // accessing buffer so lock it for mutal exclusion use
        lock_acquire(buffer_lock);
        item_buffer[write_head] = item;
        write_head = (write_head + 1) % BUFFER_SIZE;
        // finished using buffer therefore release the lock
        lock_release(buffer_lock);
        // signals the full semaphore 
        V(full);
}

/* Perform any initialisation (e.g. of global data or synchronisation
   variables) you need here. Note: You can panic if any allocation
   fails during setup
*/

void producerconsumer_startup(void)
{
        write_head = read_tail = 0;
        // create all the locks and sems
        buffer_lock = lock_create("buffer_lock");
        if(buffer_lock == NULL){
            panic("Buffer Lock fail to allocate memory");
        }
        full = sem_create("full", 0);
        if(full == NULL){
            panic("full_sem fail to allocate memory");
        }
        empty = sem_create("empty", BUFFER_SIZE);
        if(empty == NULL){
            panic("empt_sem fail to allocate memory");
        }
}

/* Perform your clean-up here */
void producerconsumer_shutdown(void)
{
        // clean up by freeing the locks and sems
        sem_destroy(full);
        lock_destroy(buffer_lock);
        sem_destroy(empty);
}

