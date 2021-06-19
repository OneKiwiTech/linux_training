#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include<signal.h>

#include "fifo.h"
#include "list.h"

typedef unsigned long uint32_t ;
#define  NUMBER_OF_CART              20
#define  NUMBER_OF_SCANNER           10

#define  MAX_CASHIER_QUEUE          (3)
#define  MAX_SCANNER_QUEUE          (2)

#define  DEFAULT_CASHIER_CHECKOUT_PERIOD       (50)
#define  DEFAULT_SCANNER_CHECKOUT_PERIOD       (20)

sem_t cart_sem;
sem_t scanner_sem;

uint32_t  id_cnt = 0;

// Declare list object
struct list_object_struct return_cart_list;
struct list_object_struct shopping_list;

// Declare FIFO object
char cashier_fifo_mem[MAX_CASHIER_QUEUE][FIFO_MAX]; // Max 3 cashier
struct fifo_obj manual_checkout_fifo[MAX_CASHIER_QUEUE3];

void *thread_function_01(void *arg);
void *thread_function_02(void *arg);
void *monitor_thread(void *arg);


int main()
{
    int res;
    pthread_t a_thread[3];
    void *thread_result;

    res = sem_init(&cart_sem, 0, NUMBER_OF_CART);
    if (res != 0) {
        perror("cart_sem initialization failed");
        exit(EXIT_FAILURE);
    }

    res = sem_init(&scanner_sem, 0, NUMBER_OF_SCANNER);
    if (res != 0) {
        perror("scanner_sem initialization failed");
        exit(EXIT_FAILURE);
    }

    //=================Init FIFO=======================

    //=================Init link list==================
    // Init return cart list
    return_cart_list.meta_data = 20;  // 20 * 100ms = 2 seconds
    create_list(&return_cart_list);

    /* Create thread API */
    res = pthread_create(&a_thread[0], NULL, thread_function_01, (void *)NULL);
    if (res != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
   
    res = pthread_create(&a_thread[1], NULL, thread_function_02, (void *)&cashier_queue_01);
    if (res != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
 #if 0
    
    res = pthread_create(&a_thread[2], NULL, monitor_thread, (void *)NULL);
    if (res != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    #endif

    printf("Waiting for thread to finish...\n");
    pthread_join(a_thread[0], &thread_result);
    pthread_join(a_thread[1], &thread_result);
    pthread_join(a_thread[2], &thread_result);
    sem_destroy(&cart_sem);

    printf("Thread joined, it returned %s\n", (char *)thread_result);

    exit(EXIT_SUCCESS);
}

/*================================PRODUCER======================================*/
/**

1. Every 2 seconds a customer arrives and takes a cart to go shopping, there are 20
carts, if no cart is available, the customer goes back home empty handed.

2. About half the customers take a handheld scanner with them, there are 10 handheld
scanners, if none are available, the customer does the shopping the normal way.

3. Shopping (and if applicable scanning with the handheld scanner) takes around 5
seconds.

4. Customers without a handheld scanner go to one of three cashiers
*/
#define MAX_ENTRY_COUNTER      20  // 20 * 100ms = 2000ms = 2s

void sig_handler(int signum) 
{
  bool has_canner = false;

  printf("Inside handler function\n");
  if (sem_trywait(&cart_sem) < 0)
  {
    printf("Empty cart, go backhome!!\r\n");
  }else 
  {
    // Add to shopping queue
    /* 
    About half the customers take a handheld scanner with them, there are 10 handheld
    scanners
    */
    has_canner = (sem_trywait(&scanner_sem) < 0)?true:false; 
    add_to_list(&shopping_list, id_cnt, has_canner);
    id_cnt++;
  }

  alarm(2);
}

void  customer_prepare_checkout(int id)
{
    printf("Customer %d need checkout\r\n", id);

    // Add to fifo
    if (fifo_data_isfull())
    {
        printf("FIDO is full\r\n");
        exit(0);
    }

    fifo_push(id);
    add_to_list(&return_cart_list, id);
}

void *thread_function_01(void *arg)
{
    bool run = true;
    int  finish_id = 0;

    printf("THREAD 01 is running. Argument was %s\n", (char *)arg);
    signal(SIGALRM,sig_handler);

    alarm(2);
    while(run)
    {
        finish_id = list_count_down();
        if ( finish_id > 0)
        {
            customer_prepare_checkout(finish_id);
        }
        usleep(100*1000); // 100ms
    }

    pthread_exit("THREAD 01 is exited");
}

/*================================CONSUMER======================================*/
void *thread_function_02(void *arg)
{
    bool run = true;
    uint32_t sem_val = 0;
    printf("THREAD 02 is running. Argument was %s\n", (char *)arg);
    while(run)
    {
        if ( fifo_data_isavailable() )
        {
            fifo_pull();
            usleep(2000*1000);
            printf("ID %d checkout DONE \r\n");
        }
        usleep(100*1000);
    }

    pthread_exit("THREAD 02 is exited");
}

/*================================RETURN CART======================================*/
/* 
After paying, every customer takes 2 seconds to bring the groceries to their bicycle or
car and return the cart, which then becomes available for new customers, and then
they go back home.
*/

void *thread_function_03(void *arg)
{
    bool run = true;
    int  finish_id = 0;

    printf("THREAD 03 is running. Argument was %s\n", (char *)arg);

    while(run)
    {
        finish_id = list_count_down(&return_cart_list);
        if ( finish_id > 0)
        {
            sem_post(&cart_sem);
        }
        usleep(100*1000); // 100ms
    }

    pthread_exit("THREAD 03 is exited");
}


void *monitor_thread(void *arg)
{
    bool run = true;
    uint32_t sem_val = 0;

    printf("MONITOR THREAD is running. Argument was %s\n", (char *)arg);
    while(run)
    {
        sleep(0.5);
        //printf("COUNTER 01 = %d, COUNTER 02 = %d\r\n", counter_01, counter_02);
        sem_getvalue(&cart_sem, &sem_val);
        system("cls");
        printf("SEMAPHORE = %d\r\n", sem_val);
    }

    pthread_exit("THREAD 02 is exited");
}

//=====================================================================
/*
- Customers without a handheld scanner go to one of three cashiers, 
- if there are already customers waiting in line for a cashier, they choose the cashier with the least
(or none) customers in line and join the end of that queue.


*/
void manual_checkout_add_queue()
{
    //TODO: chost least or non customer in line

    // Wait 5 second to register 
}

void *manual_checkout_thread(void *arg)
{
    int i = 0;
    uint32_t counter = 0;
    struct fifo_obj* obj = NULL;
    while(1)
    {
        for (i = 0; i < MAX_CASHIER_QUEUE; i++)
        {
            obj = &manual_checkout_fifo[i];
            if ( fifo_data_isavailable(obj) )
            {
                fifo_pull(obj);
            }
        }
        usleep(100*1000); // 100ms
    }
}
/* 
- When it is their turn, it takes 5 seconds to register all the items and pay.
*/

/* 
- Customers with a handheld scanner go to one of two checkout terminals, 
- if none are available, they wait in one queue, first customer in the queue takes the first terminal which becomes available. 
- Registering the handheld scanner and paying only takes 2 seconds, 
- after that the handheld scanner becomes immediately available for new
customers.
*/
