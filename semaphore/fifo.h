#ifndef FIFO_H_
#define FIFO_H_

int fifo_data_isavailable();
int fifo_data_isfull();
int fifo_push(char data);
char fifo_pull(void);

#endif /* FIFO_H_ */