/* This file will contain your solution. Modify it as you wish. */
#include <types.h>
#include <lib.h>
#include <synch.h>
#include "cafe.h"

/* Some variables shared between threads */

static unsigned int ticket_counter; /* the next customer's ticket */
static unsigned int next_serving;   /* the barista's next ticket to serve */
static unsigned current_customers;  /* customers remaining in cafe */
// the cv for all customers and barista
struct cv *cv_cust[NUM_CUSTOMERS];
struct cv *cv_bar[NUM_BARISTAS];
// array to keep track of the ticket number of customers and barista
unsigned long customer[NUM_CUSTOMERS];
unsigned long barista[NUM_BARISTAS];
// all the locks
struct lock *ticket_lock;
struct lock *serve_lock;
struct lock *customer_lock;
struct lock *table_lock;

/*
 * get_ticket: generates an ticket number for a customers next order.
 */

unsigned int get_ticket(void)
{
        unsigned int t;
        // lock the ticket
        lock_acquire(ticket_lock);
        ticket_counter = ticket_counter + 1;
        t = ticket_counter;
        lock_release(ticket_lock);

        return t;
}

/*
 * next_ticket_to_serve: generates the next ticket number for a
 * barista for that barista to serve next.
 */

unsigned int next_ticket_to_serve(void)
{
        unsigned int t;
        // lock the serving ticket
        lock_acquire(serve_lock);
        next_serving = next_serving + 1;
        t = next_serving;
        lock_release(serve_lock);

        return t;
}

/*
 * leave_cafe: a function called by a customer thread when the
 * specific thread leaves the cafe.
 */

void leave_cafe(unsigned long customer_num)
{
        (void)customer_num;
        // lock the customer decrement
        lock_acquire(customer_lock);
        current_customers = current_customers - 1;
        lock_release(customer_lock);

}


/*
 * wait_to_order() and announce_serving_ticket() work together to
 * achieve the following:
 *
 * A customer thread calP(empty);ling wait_to_order will block on a synch primitive
 * until announce_serving_ticket is called with the matching ticket.
 *
 * A barista thread calling announce_serving_ticket will block on a synch
 * primitive until the corresponding ticket is waited on, OR there are
 * no customers left in the cafe.
 *
 * wait_to_order returns the number of the barista that will serve
 * the calling customer thread.
 *
 * announce_serving_ticket returns the number of the customer that the
 * calling barista thread will serve.
 */

unsigned long wait_to_order(unsigned long customer_number, unsigned int ticket)
{
        // the barista_number, big number for debugging purpose
        unsigned long barista_number = 200000000;
        // lock the customer and barista
        lock_acquire(table_lock);
        // set the assign ticket to customer
        customer[customer_number] = ticket;
        // look for barista with that ticket
        for(int i = 0; i <= NUM_BARISTAS;i++){
            // no barista with ticket therefore we wait
            if(i == NUM_BARISTAS){
                cv_wait(cv_cust[customer_number], table_lock);
                // someone signalled therefore we find the barista number
                for(int l = 0; l < NUM_BARISTAS;l++){
                    if(barista[l] == ticket){
                        barista_number = l;
                    }
                }
            }
            // we found a barista with the ticket
            else if(barista[i] == ticket){
                barista_number = i;
                // signal the barista that we have the ticket if waiting
                cv_signal(cv_bar[i], table_lock);
                break;
            }
        }
        lock_release(table_lock);
        return barista_number;
}

unsigned long announce_serving_ticket(unsigned long barista_number, unsigned int serving)
{
        // the customer number, big number for debugging purpose
        unsigned long cust = 100000000;

        lock_acquire(table_lock);
        // assign serving number to barista
        barista[barista_number] = serving;
        // find customer with the number
        for(int j = 0; j <= NUM_CUSTOMERS;j++){
            // no customer with the number therefore we wait
            if(j == NUM_CUSTOMERS){
                cv_wait(cv_bar[barista_number], table_lock);
                // someone signalled the number we now find the customer
                for(int k = 0; k < NUM_CUSTOMERS;k++){
                    if(customer[k] == serving){
                        cust = k;
                    }
                }
            }
            // we found a customer with the ticket
            else if(customer[j] == serving){
                cust = j;
                // signal the customer
                cv_signal(cv_cust[j], table_lock);
                break;
            }
        }
        lock_release(table_lock);
        return cust;
}

/* 
 * cafe_startup: A function to allocate and/or intitialise any memory
 * or synchronisation primitives that are needed prior to the
 * customers and baristas arriving in the cafe.
 */
void cafe_startup(void)
{

        ticket_counter = 0;
        next_serving = 0;
        current_customers = NUM_CUSTOMERS;
        // Make all the locks and then panic if cant allocate any
        ticket_lock = lock_create("ticket lock");
        if(ticket_lock == NULL){
            panic("ticket lock failed to allocate");
        }
        table_lock = lock_create("table lock");
        if(table_lock == NULL){
            panic("table lock failed to allocate");
        }
        serve_lock = lock_create("serve lock");
        if(serve_lock == NULL){
            panic("serve lock failed to allocate");
        }
        customer_lock = lock_create("customer lock");
        if(customer_lock == NULL){
            panic("customer lock failed to allocate");
        }
        // create a cv for all the customer and barista
        for(int i = 0; i < NUM_CUSTOMERS; i++){
            cv_cust[i] = cv_create("customer");
        }
        for(int i = 0; i < NUM_BARISTAS; i++){
            cv_bar[i] = cv_create("barista");
        }

}   

/*
 * cafe_shutdown: A function called after baristas and customers have
 * exited to de-allocate any memory or synchronisation
 * primitives. Anything allocated during startup should be
 * de-allocated after calling this function.
 */

void cafe_shutdown(void)
{
        // destroy all lock and cv, no free function for array
        lock_destroy(ticket_lock);
        lock_destroy(serve_lock);
        lock_destroy(customer_lock);
        lock_destroy(table_lock);
        for(int i = 0; i < NUM_CUSTOMERS; i++){
            cv_destroy(cv_cust[i]);
        }
        for(int i = 0; i < NUM_BARISTAS; i++){
            cv_destroy(cv_bar[i]);
        }
}
                              
