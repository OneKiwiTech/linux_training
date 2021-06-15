#include <stdio.h>
#include <stdlib.h>

char fifo[256];
int fifo_tail;
int fifo_head;
int fifo_n_data;

#define FIFO_MAX 256;



int fifo_data_isavailable()
{
  if (fifo_n_data > 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }

}

int fifo_data_isfull()
{
  if (fifo_n_data < 256)
    return 0;
  else
    return 1;
}

int fifo_push(char data)
{
  if (!fifo_data_isfull())
  {
    fifo[fifo_head] = data;
    if (fifo_head < 255)
    {
      fifo_head ++;
    }
    else
    {
      fifo_head = 0;
    }

    fifo_n_data ++;
    return 1;
  }
  else
  {
    return 0;
  }

}

char fifo_pull(void)
{
  char data;
  if(fifo_data_isavailable())
  {
    data = fifo[fifo_tail];
    if (fifo_tail < 255)
    {
      fifo_tail ++;
    }
    else
    {
      fifo_tail = 0;
    }
    fifo_n_data --;
    return data;
  }
  return -1;
}