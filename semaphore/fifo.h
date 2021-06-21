#ifndef FIFO_H_
#define FIFO_H_

#define FIFO_MAX            256

struct fifo_obj
{
  char* mem_pool;
  int fifo_tail;
  int fifo_head;
  int fifo_n_data;
};

int fifo_data_isavailable(struct fifo_obj* obj);
int fifo_data_isfull(struct fifo_obj* obj);
int fifo_push(struct fifo_obj* obj, int data);
char fifo_pull(struct fifo_obj* obj);

#endif /* FIFO_H_ */