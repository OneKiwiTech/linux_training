#include <fcntl.h>
#include <stdio.h>

const char* test_string = "0123456789ABCDEF";

char led_segment_read(int fd)
{
  char buf[4];
  read(fd, buf, sizeof(buf));

  return buf[0];
}

int led_segment_write(int fd, char data)
{
    char wbuf[4];

    wbuf[0] = 1;
    wbuf[1] = data;
    
    return ( write(fd, wbuf, sizeof(wbuf)) );
}


int main (void)
{
  int fd;
  char* str_ptr = NULL;
  
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
      printf("Read back data %d", led_segment_read(fd) );
      delay (500) ;
    }
  }

  close(fd);

  return 0 ;
}