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

typedef unsigned long uint32_t;
typedef unsigned int uint16_t;

#define  NUMBER_OF_CART              20
#define  NUMBER_OF_SCANNER           10

#define  MAX_CASHIER_QUEUE          (3)
#define  MAX_SCANNER_QUEUE          (2)
#define  MAX_TOTAL_QUEUE            (MAX_CASHIER_QUEUE + MAX_SCANNER_QUEUE)

#define  DEFAULT_CASHIER_CHECKOUT_PERIOD       (50)
#define  DEFAULT_SCANNER_CHECKOUT_PERIOD       (20)

#define MAX_ENTRY_COUNTER      20  // 20 * 100ms = 2000ms = 2s

static sem_t cart_sem;
static sem_t scanner_sem;

static uint32_t  id_cnt = 0;

// Declare list object
struct list_object_struct return_cart_list;
struct list_object_struct shopping_list;

// Declare FIFO object
char cashier_fifo_mem[MAX_TOTAL_QUEUE][FIFO_MAX]; 
struct fifo_obj manual_checkout_fifo[MAX_CASHIER_QUEUE];
struct fifo_obj scanner_checkout_fifo[MAX_SCANNER_QUEUE];

void scanner_checkout_add_to_queue(char id);
void manual_checkout_add_to_queue(char id);

void *thread_shopping_tracking(void *arg);
void *scanner_checkout_thread(void *arg);
void *manual_checkout_thread(void *arg);
void *thread_return_cart(void *arg);

int main()
{
    int res;
    int i = 0;

    pthread_t shopping_thread;
    pthread_t cashier_thread[3];
    pthread_t scanner_thread[2];
    pthread_t return_thread;

    void *thread_result;

    // ================= INIT SEMAPHORE ==============
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

    

    //=================Init link list===================
    // Init return cart list
    return_cart_list.meta_data = 20;  // 20 * 100ms = 2 seconds
    create_list(&return_cart_list, 0);

    shopping_list.meta_data = 50;  // 20 * 100ms = 2 seconds
    create_list(&shopping_list, 0);
    
    //=================Init THREAD=======================
    res = pthread_create(&shopping_thread, NULL, thread_shopping_tracking, (void *)NULL);
    if (res != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
   
    res = pthread_create(&return_thread, NULL, thread_return_cart, (void *)NULL);
    if (res != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < 3; i++)
    {
        res = pthread_create(&cashier_thread[i], NULL, manual_checkout_thread, (void *)&manual_checkout_fifo[i]);
        if (res != 0)
        {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < 2; i++)
    {
        res = pthread_create(&scanner_thread[i], NULL, scanner_checkout_thread, (void *)&scanner_checkout_fifo[i]);
        if (res != 0)
        {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // Waiting all thread finish
    printf("Waiting for thread to finish...\n");
    pthread_join(shopping_thread, &thread_result);
    pthread_join(return_thread, &thread_result);
    for (i = 0; i < 3; i++)
    {
        pthread_join(cashier_thread[i], &thread_result);
    }

    for (i = 0; i < 2; i++)
    {
        pthread_join(scanner_thread[i], &thread_result);
    }

    // Destroy resources
    sem_destroy(&cart_sem);
    sem_destroy(&scanner_sem);

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
    has_canner = (sem_trywait(&scanner_sem) > 0)?true:false; 
    add_to_list(&shopping_list, id_cnt, has_canner);
    id_cnt++;
  }

  alarm(2);
}

void  customer_prepare_checkout(struct customer_info_obj *obj)
{
    printf("Customer %d need checkout\r\n", obj->val);

    //TODO: choose cashier or scanner queue
    if (obj->has_scanner)
    {
        scanner_checkout_add_to_queue(obj->val);
    }else 
    {
        manual_checkout_add_to_queue(obj->val);
    }
}

void *thread_shopping_tracking(void *arg)
{
    bool run = true;
    int  finish_id = 0;
    struct customer_info_obj *curr_cust_obj;

    printf("THREAD 01 is running. Argument was %s\n", (char *)arg);
    signal(SIGALRM,sig_handler);

    alarm(2);
    while(run)
    {
        finish_id = list_count_down(&shopping_list, curr_cust_obj);
        if ( finish_id > 0)
        {
            customer_prepare_checkout(curr_cust_obj);
        }
        usleep(100*1000); // 100ms
    }

    pthread_exit("THREAD 01 is exited");
}

/*================================RETURN CART======================================*/
/* 
After paying, every customer takes 2 seconds to bring the groceries to their bicycle or
car and return the cart, which then becomes available for new customers, and then
they go back home.
*/

void *thread_return_cart(void *arg)
{
    bool run = true;
    int  finish_id = 0;
    struct customer_info_obj *obj = NULL;

    printf("THREAD 03 is running. Argument was %s\n", (char *)arg);

    while(run)
    {
        finish_id = list_count_down(&return_cart_list, obj);
        if ( finish_id > 0)
        {
            sem_post(&cart_sem);
        }
        usleep(100*1000); // 100ms
    }

    pthread_exit("THREAD 03 is exited");
}


//===========================CASHIER CHECKOUT==========================================
/*
- if there are already customers waiting in line for a cashier, 
  they choose the cashier with the least (or none) customers in line and join the end of that queue.
*/

void manual_checkout_add_to_queue(char id)
{
    int i = 0; 
    struct fifo_obj* obj = NULL;
    int curr_queue_cnt = 0;
    int curr_queue_idx = -1;

    // Find cashier with the least or none customer
    for (i = 0; i < 3; i++)
    {
        obj = &manual_checkout_fifo[i];
        if (obj->fifo_n_data <= curr_queue_cnt);
        {
            curr_queue_cnt = obj->fifo_n_data;
            curr_queue_idx = i;
        }
    }

    fifo_push(obj, id); 
}




/* 
- When it is their turn, it takes 5 seconds to register all the items and pay.
*/
void *manual_checkout_thread(void *arg)
{
    struct fifo_obj* obj = (struct fifo_obj*)arg;
    bool checkout_turn = true;
    uint32_t counter = DEFAULT_CASHIER_CHECKOUT_PERIOD;
    int id = 0;

    while(1)
    {
        // Check & pull customer from queue 
        if (checkout_turn)
        {
            if ( fifo_data_isavailable(obj) )
            {
                id = fifo_pull(obj);
                checkout_turn = false;
            }
        }else 
        {
            if (counter == 0)
            {
                checkout_turn = true;
                // Add to return cart list
                add_to_list(&return_cart_list, id, false);
            }
            counter -= 1;
        }
        usleep(100*1000); // 100ms
    }
}


//==============================================================================================
/* 
- Customers with a handheld scanner go to one of two checkout terminals, 
- if none are available, they wait in one queue, first customer in the queue takes the first terminal which becomes available. 
*/
void scanner_checkout_add_to_queue(char id)
{
    int i = 0; 
    struct fifo_obj* obj = NULL;
    int curr_queue_cnt = 0;
    int curr_queue_idx = -1;

    // Find cashier with the least or none customer
    for (i = 0; i < 2; i++)
    {
        obj = &scanner_checkout_fifo[i];
        if (obj->fifo_n_data <= curr_queue_cnt);
        {
            curr_queue_cnt = obj->fifo_n_data;
            curr_queue_idx = i;
        }
    }

    fifo_push(obj, id);
}

/*
- Registering the handheld scanner and paying only takes 2 seconds, 
- after that the handheld scanner becomes immediately available for new
customers.
*/
void *scanner_checkout_thread(void *arg)
{
    struct fifo_obj* obj = (struct fifo_obj*)arg;
    bool checkout_turn = true;
    uint32_t counter = DEFAULT_SCANNER_CHECKOUT_PERIOD;
    int id = 0;

    while(1)
    {
        // Check & pull customer from queue 
        if (checkout_turn)
        {
            if ( fifo_data_isavailable(obj) )
            {
                id = fifo_pull(obj);
                checkout_turn = false;
            }
        }else 
        {
            if (counter == 0)
            {
                checkout_turn = true;
                // Release scanner for next person
                sem_post(&scanner_sem);
                // Add to return cart list
                add_to_list(&return_cart_list, id, true);
            }
            counter -= 1;
        }
        usleep(100*1000); // 100ms
    }
}