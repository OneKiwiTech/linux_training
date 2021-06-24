#include <fcntl.h>
#include <stdio.h>

const char* test_string = "0123456789ABCDEF";

char led_segment_read(int fd, char* user_buf, int len)
{
  read(fd, user_buf, len);

  return len;
}

int led_segment_write(int fd, char data)
{
    char wbuf[2] = {0};

    wbuf[0] = data;

    return ( write(fd, wbuf, sizeof(wbuf)) );
}


int main (void)
{
  int fd;
  char* str_ptr = NULL;
  char read_buf[2] = {0};
  int len = 0;

  fd = open("/dev/io_dev", O_RDWR);

  if (fd < 0)
  {
    printf("open gpio device error!\r\n");
    exit(-1);
  }

  for (;;)
  {
    str_ptr = &test_string[0];
    while(*str_ptr != NULL)
    {
      led_segment_write(fd, *str_ptr++);

      led_segment_read(fd, read_buf, 1);
      printf("Read back data %d", read_buf[0]);
      delay (500) ;
    }
  }

  close(fd);

  return 0 ;
}