#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include<signal.h>

#include "fifo.h"

typedef unsigned long uint32_t ;
#define  DEFAULT_SEM_COUNTER_VAL           20

sem_t bin_sem;
uint32_t  id_cnt = 0;

void *thread_function_01(void *arg);
void *thread_function_02(void *arg);
void *monitor_thread(void *arg);

typedef struct
{
    uint32_t in_person_count;
    uint32_t remain_time;
    uint32_t out_person_count;
} cashier_queue_t;

cashier_queue_t cashier_queue_01;

int main()
{
    int res;
    pthread_t a_thread[3];
    void *thread_result;

    res = sem_init(&bin_sem, 0, DEFAULT_SEM_COUNTER_VAL);
    if (res != 0) {
        perror("Semaphore initialization failed");
        exit(EXIT_FAILURE);
    }

    // Init link list
    

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
    sem_destroy(&bin_sem);

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

void sig_handler(int signum){
 
  printf("Inside handler function\n");
  if (sem_trywait(&bin_sem) < 0)
  {
    printf("Empty cart, go backhome!!\r\n");
  }else 
  {
    // Add to shopping queue
    add_to_list(id_cnt, true);
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


void *monitor_thread(void *arg)
{
    bool run = true;
    uint32_t sem_val = 0;

    printf("MONITOR THREAD is running. Argument was %s\n", (char *)arg);
    while(run)
    {
        sleep(0.5);
        //printf("COUNTER 01 = %d, COUNTER 02 = %d\r\n", counter_01, counter_02);
        sem_getvalue(&bin_sem, &sem_val);
        system("cls");
        printf("SEMAPHORE = %d\r\n", sem_val);
    }

    pthread_exit("THREAD 02 is exited");
}
