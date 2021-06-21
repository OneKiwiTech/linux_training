// Simple Character Device Driver Module for Raspberry Pi.
/*
 * DESCRIPTION:
 *     a simple example of char device 
 *     this char device can control the GPIO by file operation : write
 *     to write specific message as command
 * */

#include <linux/module.h>   
#include <linux/string.h>    
#include <linux/fs.h>      
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <linux/list.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/time.h>
#include <linux/delay.h>

#define MY_MAJOR  200
#define MY_MINOR  0
#define MY_DEV_COUNT 1
#define MY_MAX_GPIO_COUNT  7

#define GPIO_DEVICE_DESC    "caothinh_driver"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("caothinh");
MODULE_DESCRIPTION("A Simple 7 Segment Display Device Driver module");

static int     my_open( struct inode *, struct file * );
static ssize_t my_read( struct file * ,        char *  , size_t, loff_t *);
static ssize_t my_write(struct file * , const  char *  , size_t, loff_t *);
static int     my_close(struct inode *, struct file * );
struct file_operations my_fops = {
        read    :       my_read,
        write   :       my_write,
        open    :       my_open,
        release :       my_close,
        owner   :       THIS_MODULE
};

static char   *msg=NULL;
struct cdev my_cdev;


// LED segment data
static int curr_display_data = 0;
// https://forums.ni.com/t5/Community-Documents/LabVIEW-BCM2835-Library-for-Raspberry-Pi/ta-p/3539080?profile.language=en
static int segments [7] = {  2, 3,  4,  17,  27 ,  22, 10 } ;

static const int segmentDigits [] =
{
// a  b  c  d  e  f  g     Segments
// 6  5  4  3  2  1  0,	// wiringPi pin No.

   1, 1, 1, 1, 1, 1, 0,	// 0
   0, 1, 1, 0, 0, 0, 0,	// 1
   1, 1, 0, 1, 1, 0, 1,	// 2
   1, 1, 1, 1, 0, 0, 1,	// 3
   0, 1, 1, 0, 0, 1, 1,	// 4
   1, 0, 1, 1, 0, 1, 1,	// 5
   1, 0, 1, 1, 1, 1, 1,	// 6
   1, 1, 1, 0, 0, 0, 0,	// 7
   1, 1, 1, 1, 1, 1, 1,	// 8
   1, 1, 1, 1, 0, 1, 1,	// 9
   1, 1, 1, 0, 1, 1, 1,	// A
   0, 0, 1, 1, 1, 1, 1,	// b
   1, 0, 0, 1, 1, 1, 0,	// C
   0, 1, 1, 1, 1, 0, 1,	// d
   1, 0, 0, 1, 1, 1, 1,	// E
   1, 0, 0, 0, 1, 1, 1,	// F
   0, 0, 0, 0, 0, 0, 0,	// blank
} ;

static void gpio_set_default_state();

/*
 * INIT_MODULE -- MODULE START --
 * */

int init_module(void)
{

	dev_t devno;
	unsigned int count = MY_DEV_COUNT;
	int err;
	int i = 0;

	devno = MKDEV(MY_MAJOR, MY_MINOR);
	register_chrdev_region(devno, count , "caothinh_driver");

	// -- initial the char device 
	cdev_init(&my_cdev, &my_fops);
	my_cdev.owner = THIS_MODULE;
	err = cdev_add(&my_cdev, devno, count);

	if (err < 0)
	{
		printk("Device Add Error\n");
		return -1;
	}

	// -- print message 
	printk("<1> Hello World. This is caothinh_driver Driver.\n");
	printk("'mknod /dev/caothinh_driver0 c %d 0'.\n", MY_MAJOR);

	// -- make 
	msg   = (char *)kmalloc(32, GFP_KERNEL);
	if (msg !=NULL)
	{
		printk("malloc allocator address: 0x%p\n", msg);
	}
	
	printk("***** 7 SEGMENT LED GPIO Init ******************\n");
	for (i = 0; i < MY_MAX_GPIO_COUNT; i++)
	{
		if(gpio_is_valid(segments[i]) < 0){
			printk("gpio %d is valid error \n", segments[i]);
			return -1;
		}
	}

	for (i = 0; i < MY_MAX_GPIO_COUNT; i++)
	{
		if(gpio_request(segments[i], "LED_GPIO") < 0){
			printk("gpio %d is request error \n", segments[i]);
			return -1;
		}
	}

	for (i = 0; i < MY_MAX_GPIO_COUNT; i++)
	{
		gpio_direction_output(segments[i], 0);
	}

    return 0;
}


static void segment_disp_digit(int d)
{
 	int segment ;
 	int index, d, segVal ;

	for (segment = 0 ; segment < 7 ; ++segment)
	{
		if ((d >= '0') && (d <= '9'))	// Digit
		{
			index = d - '0' ;
		}else if ((d >= 'A') && (d <= 'F'))	// Hex
		{
			index = d - 'A' + 10 ;
		}else
		{
			index = 16 ;				// Blank
		}

		segVal = segmentDigits [index * 7 + segment] ;
		gpio_set_value (segments [segment], segVal) ;
	}
}

/*
 * CLEANUP_MODULE -- MODULE END --
 * */
void cleanup_module(void)
{
	dev_t devno;
	int i;
    printk("<1> Goodbye\n");

	for (i = 0; i < MY_MAX_GPIO_COUNT; i++)
	{
		gpio_set_value(segments[i],0);
	}
	
	for (i = 0; i < MY_MAX_GPIO_COUNT; i++)
	{
		gpio_free(segments[i],0);
	}
	devno = MKDEV(MY_MAJOR, MY_MINOR);
	if (msg){
        /* release the malloc */
        kfree(msg);
	}

	unregister_chrdev_region(devno, MY_DEV_COUNT);
	cdev_del(&my_cdev);
}


/*
 * file operation: OPEN 
 * */
static int my_open(struct inode *inod, struct file *fil)
{
	int major;
	int minor;

    major = imajor(inod);
    minor = iminor(inod);
    printk("\n*****Driver major %d  minor %d*****\n",major, minor);

    return 0;
}




/*
 * file operation: READ
 * */
static ssize_t my_read(struct file *filp, char *buff, size_t len, loff_t *off)
{
	char led_value = 0;
	short count;

	for (i = 0; i < MY_MAX_GPIO_COUNT; i++)
	{
		led_value |= gpio_get_value(segments[i]);
		led_value <<= 1;
	}

	// Copy to user buffer
	msg[0] = led_value;
	len = 1;
	count = copy_to_user(buff, msg, len);
	printk("Segment value = %d\n", led_value);

	return count;
}


/*
 * file operation: WRITE
 * */
static ssize_t my_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	short count = 0;

	count = copy_from_user( &curr_display_data, buff, 1 );

	printk("Receive data %d len %d\n", curr_display_data, len);

	return len;
}



/*
 * file operation : CLOSE
 * */
static int my_close(struct inode *inod, struct file *fil)
{
	printk("*****Device driver is closed \n");

	return 0;
}