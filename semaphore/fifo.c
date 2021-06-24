#include <stdio.h>
#include <stdlib.h>
#include "fifo.h"

void fifo_init(struct fifo_obj* obj)
{
  obj->fifo_tail = 0; 
  obj->fifo_head = 0; 
  obj->fifo_n_data = 0; 
}

int fifo_data_isavailable(struct fifo_obj* obj)
{
  if (obj->fifo_n_data > 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }

}

int fifo_data_isfull(struct fifo_obj* obj)
{
  if (obj->fifo_n_data < FIFO_MAX)
  {
    return 0;
  }else
  {
    return 1;
  }
}

int fifo_push(struct fifo_obj* obj, int data)
{
  if (!fifo_data_isfull(obj))
  {
    printf("Push %d\n", data);
    obj->mem_pool[obj->fifo_head] = data;
    if (obj->fifo_head < 255)
    {
      obj->fifo_head++;
    }
    else
    {
      obj->fifo_head = 0;
    }

    obj->fifo_n_data++;
    return 1;
  }
  else
  {
    printf("FIDO is FULL\n");
    return 0;
  }

}

char fifo_pull(struct fifo_obj* obj)
{
  char data;
  if(fifo_data_isavailable(obj))
  {
  
    data = obj->mem_pool[obj->fifo_tail];
    printf("==>PULL %d\n", data);
    if (obj->fifo_tail < 255)
    {
      obj->fifo_tail ++;
    }
    else
    {
      obj->fifo_tail = 0;
    }
    obj->fifo_n_data --;

    return data;
  }
  
  return -1;
}