#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include<signal.h>

#include "fifo.h"
#include "my_list.h"

// #define   PROJ_USE_RANDOM_ENTER_TIME  
// #define   PROJ_USE_RANDOM_SHOPPING_TIME 
// #define   PROJ_USE_RANDOM_CHECKOUT_TIME 
#define TEST_MODE


#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( false )
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif

#define MONITOR_PRINT(...) do{ fprintf( stdout, __VA_ARGS__ ); } while( false )

typedef unsigned long uint32_t;
typedef unsigned int uint16_t;


#define   DEFAULT_SHOPPING_CHECKED_TIME       (5)
#define   DEFAULT_SHOPPING_CHECKED_PERCENT    (25)

#define  DEFAULT_CUSTOMER_ENTER_PERIOD         (2) //second
#define  NUMBER_OF_CART              20
#define  NUMBER_OF_SCANNER           10

#define  MAX_CASHIER_QUEUE          (3)
#define  MAX_SCANNER_QUEUE          (2)
#define  MAX_TOTAL_QUEUE            (MAX_CASHIER_QUEUE + MAX_SCANNER_QUEUE)

#ifdef TEST_MODE
    #define  DEFAULT_CASHIER_CHECKOUT_PERIOD       (5)
    #define  DEFAULT_SCANNER_CHECKOUT_PERIOD       (2)
    #define  DEFAULT_SHOPPING_PERIOD               (5)
    #define  DEFAULT_RETURN_CART_PERIOD            (2)
#else
    #define  DEFAULT_CASHIER_CHECKOUT_PERIOD       (50)
    #define  DEFAULT_SCANNER_CHECKOUT_PERIOD       (20)
    #define  DEFAULT_SHOPPING_PERIOD               (50)
    #define  DEFAULT_RETURN_CART_PERIOD            (20)
#endif

#define MAX_ENTRY_COUNTER      20  // 20 * 100ms = 2000ms = 2s

static sem_t cart_sem;
static sem_t scanner_sem;

static uint32_t  customer_enter_time = DEFAULT_CUSTOMER_ENTER_PERIOD;

static uint32_t  id_cnt = 1;

// Declare list object
LIST_HEAD(return_cart_list);
LIST_HEAD(shopping_list);

list_object_struct_t *return_list_head = NULL;
list_object_struct_t *shopping_list_head = NULL;

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

void *thread_monitor_run(void *arg);


int main()
{
    int res;
    int i = 0;

    pthread_t shopping_thread;
    pthread_t cashier_thread[3];
    pthread_t scanner_thread[2];
    pthread_t return_thread;
    pthread_t monitor_thread;

    void *thread_result;

    // ================= INIT SEMAPHORE ==============
    res = sem_init(&cart_sem, 0, NUMBER_OF_CART);
    if (res != 0) {
        DEBUG_PRINT("cart_sem initialization failed");
        exit(EXIT_FAILURE);
    }

    res = sem_init(&scanner_sem, 0, NUMBER_OF_SCANNER);
    if (res != 0) {
        DEBUG_PRINT("scanner_sem initialization failed");
        exit(EXIT_FAILURE);
    }

    //=================Init link list===================
    // Init return cart list
    return_list_head = create_list(&return_cart_list, 1, DEFAULT_RETURN_CART_PERIOD);
    if (return_list_head == NULL)
    {
        DEBUG_PRINT("Init list failed\n");
        exit(EXIT_FAILURE);
    }
    shopping_list_head = create_list(&shopping_list, 2, DEFAULT_SHOPPING_PERIOD);
    if (shopping_list_head == NULL)
    {
        DEBUG_PRINT("Init list failed\n");
        exit(EXIT_FAILURE);
    }
    //=================Init THREAD=======================
     
    res = pthread_create(&monitor_thread, NULL, thread_monitor_run, (void *)NULL);
    if (res != 0)
    {
        DEBUG_PRINT("Thread creation failed");
        exit(EXIT_FAILURE);
    }
   

    res = pthread_create(&shopping_thread, NULL,  thread_shopping_tracking, (void *)NULL);
    if (res != 0)
    {
        DEBUG_PRINT("Thread creation failed");
        exit(EXIT_FAILURE);
    }


    res = pthread_create(&return_thread, NULL, thread_return_cart, (void *)NULL);
    if (res != 0)
    {
        DEBUG_PRINT("Thread creation failed");
        exit(EXIT_FAILURE);
    }

 #if 1
    for (i = 0; i < 3; i++)
    {
        res = pthread_create(&cashier_thread[i], NULL, manual_checkout_thread, (void *)&manual_checkout_fifo[i]);
        if (res != 0)
        {
            DEBUG_PRINT("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

  
    for (i = 0; i < 2; i++)
    {
        res = pthread_create(&scanner_thread[i], NULL, scanner_checkout_thread, (void *)&scanner_checkout_fifo[i]);
        if (res != 0)
        {
            DEBUG_PRINT("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }
    #endif

    // Waiting all thread finish
    DEBUG_PRINT("Waiting for thread to finish...\n");
    pthread_join(shopping_thread, &thread_result);
    pthread_join(return_thread, &thread_result);
    pthread_join(monitor_thread, &thread_result);
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

    DEBUG_PRINT("Thread joined, it returned %s\n", (char *)thread_result);

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

void sig_customer_enter_timer(int signum) 
{
  bool has_canner = false;

 DEBUG_PRINT("Customer enters\n");

  if (sem_trywait(&cart_sem) < 0)
  {
    DEBUG_PRINT("Empty cart, go backhome!!\r\n");
  }else 
  {
    // Add to shopping queue
    /* 
    About half the customers take a handheld scanner with them, there are 10 handheld
    scanners
    */
   
    has_canner = (sem_trywait(&scanner_sem) > 0)?true:false; 

    DEBUG_PRINT("Customer is added to shopping list!!\r\n");
    add_to_list(shopping_list_head, id_cnt, has_canner);
    id_cnt++;
  }

  alarm(customer_enter_time);
}

#define   CASHIER_QUEUE_LEN_THRESHOLD       5

void  customer_prepare_checkout(customer_info_obj_t *obj)
{
    int i = 0;
    bool found = false;

    DEBUG_PRINT("Customer %d need checkout\r\n", obj->id);

    //TODO: choose cashier or scanner queue
    if (obj->has_scanner)
    {
        scanner_checkout_add_to_queue(obj->id);
    }else 
    {
        // TODO: check queue length of cashier 
        for (i = 0; i < MAX_CASHIER_QUEUE; i++)
        {
            if (manual_checkout_fifo[i].fifo_n_data >= CASHIER_QUEUE_LEN_THRESHOLD)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            //TODO: mark this object has manual checkout time
            scanner_checkout_add_to_queue(obj->id);
        }   
        else
        {
            manual_checkout_add_to_queue(obj->id);
        }
    }
}

void *thread_shopping_tracking(void *arg)
{
    bool run = true;
    customer_info_obj_t *curr_cust_obj = NULL;

    DEBUG_PRINT("THREAD shopping is running. Argument was %s\n", (char *)arg);
    signal(SIGALRM,sig_customer_enter_timer);

    alarm(customer_enter_time);
    while(run)
    {
        curr_cust_obj = list_count_down(&shopping_list, shopping_list_head);
        if ( curr_cust_obj != NULL)
        {   
            DEBUG_PRINT("==>customer need checkout\n");
            delete_from_list(&shopping_list, shopping_list_head, curr_cust_obj);
            // customer_prepare_checkout(curr_cust_obj);
        }

#if USE_RANDOM_PERIOD
        // Random checked all customer shopping
        get_random_check_during_shopping(shopping_list.counter, DEFAULT_SHOPPING_CHECKED_PERCENT);
        curr_cust_obj->remain_time += DEFAULT_SHOPPING_CHECKED_TIME;
#endif

        usleep(100*1000); // 100ms
    }

    pthread_exit("THREAD shopping is exited");
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
    customer_info_obj_t *obj = NULL;

    DEBUG_PRINT("THREAD return cart is running. Argument was %s\n", (char *)arg);

    while(run)
    {
        obj = list_count_down(&return_cart_list, return_list_head);
        if ( obj != NULL)
        {
            sem_post(&cart_sem);
        }
        usleep(100*1000); // 100ms
    }

    pthread_exit("THREAD return cart is exited");
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
                add_to_list(return_list_head, id, false);
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
                add_to_list(return_list_head, id, true);
            }
            counter -= 1;
        }
        usleep(100*1000); // 100ms
    }
}


int random_number(int min_num, int max_num)
{
    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num)
    {
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    } else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }

    srand(time(NULL));
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

// https://stackoverflow.com/questions/26892104/selecting-a-random-number-in-a-set-of-numbers-in-c
static int get_random_check_during_shopping(int max_num, int percent)
{
   int  range =  (max_num*percent) / 100;
   
   int min = ( rand() % max_num ); //random number 0-20        
   int r = ( rand() % (range-min) ) + min; //random number will be greater than min but less than range 

   return r;
}

static  uint32_t get_random_enter_time()
{
#ifdef PROJ_USE_RANDOM_ENTER_TIME
   return random_number(1, 5);
#else
   return DEFAULT_CUSTOMER_ENTER_PERIOD;
#endif   
}

static  uint32_t get_random_shopping_time()
{
#ifdef PROJ_USE_RANDOM_SHOPPING_TIME    
    random_number(2, 20);
#else
    return DEFAULT_SHOPPING_PERIOD;
#endif
}

static  uint32_t get_random_cashier_checkout_time()
{
#ifdef PROJ_USE_RANDOM_CHECKOUT_TIME    
    random_number(20, 70);
#else
    return DEFAULT_CASHIER_CHECKOUT_PERIOD;
#endif
}

static  uint32_t get_random_scanner_checkout_time()
{
#ifdef PROJ_USE_RANDOM_CHECKOUT_TIME    
    random_number(10, 40);
#else
    return DEFAULT_SCANNER_CHECKOUT_PERIOD;
#endif
}

void* thread_monitor_run(void* arg)
{
    while(1)
    {
        DEBUG_PRINT("Customer count = %d\n", id_cnt);
        usleep(1000*1000);
        // system("cls");
    }

    return NULL;
}