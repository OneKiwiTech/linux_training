#include <fcntl.h>



int gpio_init()
{
    int fd;
fd = open("/dev/io_dev", O_RDWR);
return fd;

}
void gpio_read(int fd)
{
    char buf[4];
char wbuf[4];
read(fd, buf, sizeof(buf));
}
void gpio_set(int fd)
{
    wbuf[0] = 1;
    wbuf[1] = 0;
    write(fd, wbuf, sizeof(wbuf));
}

void gpio_clear(int fd)
{
    wbuf[0] = 0;
    wbuf[1] = 0;
    write(fd, wbuf, sizeof(wbuf));

}
int main()
{
    close(fd);
}