/*

 https://www.codeproject.com/Articles/1032794/Simple-I-O-device-driver-for-RaspberryPi
sudo apt-get install linux-headers-$(uname -r)

 */
#include "linux/version.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/ioport.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

#define SUCCESS 0
#define DEVICE_NAME "io_dev" // Dev name as it appears in /proc/devices   
#define BUF_LEN 80		     // Max length of the message from the device 

static unsigned PORT = 0x20200000; 

static unsigned RANGE =  0x40;
unsigned cmd_word;
unsigned out_word;
u8 *addr;

static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;


static int segments [7] = {  6,  5,  4,  3,  2,  1, 0 } ;
static int digit_pin = 5; 

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

static int device_open(struct inode *inode, struct file *file)
{
	u32 cmd;   // command word to write
	msg_Ptr = msg;
	sprintf(msg,"Not implemented\n");
	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	try_module_get(THIS_MODULE);
	addr = ioremap(PORT, RANGE);
	if (addr  != NULL)
	{
		cmd = 0;
		writel(cmd, (addr+4)); // clear the setting of pin 10
		cmd = 1;
		writel(cmd, (addr+4)); // set pin 10 as output

		//TODO: config output 7 segment GPIO
		return SUCCESS;
    }
    else
      return -ENODEV;

}

static int device_release(struct inode *inode, struct file *file)
{
	writel(0x00, (addr+4));  // claer the setting before release
	Device_Open--;		     //make ready for the next caller
	module_put(THIS_MODULE);

	return 0;
}

static ssize_t device_read(struct file *filp,	
			   char *buffer,	
			   size_t length,	
			   loff_t * offset)
{
    int bytes_read = 1;
    int index;
    u32 res;   // status word to read
	msg_Ptr = msg;

	msg[0] = (res >> 24) & 0xFF;
	put_user(*(msg_Ptr++), buffer++);

	return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int bytes_written = 1;
	u32 cmd;   // command word to write
	if (buff)
	{
		if (buff[0] == 1)
		{
			cmd = 1 << 10;
			writel(cmd, (addr+0x1c));
		}
		else
		{
			if (buff[0] == 0)
			{
				cmd = 1 << 10;
				writel(cmd, (addr+0x28));
			}
		}
	}
	return bytes_written;
}

static void gpio_set_pin_state(int pin_no, int value)
{
	u32 cmd;   // command word to write
	if (buff)
	{
		if (buff[0] == 1)
		{
			cmd = 1 << 10;
			writel(cmd, (addr+0x1c));
		}
		else
		{
			if (buff[0] == 0)
			{
				cmd = 1 << 10;
				writel(cmd, (addr+0x28));
			}
		}
	}
}

static void displayDigits(int d)
{
  int segment ;
  int index, d, segVal ;

	gpio_set_pin_state (digit_pin, 1) ;
	
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
		gpio_set_pin_state (segments [segment], segVal) ;
	}

    gpio_set_pin_state (digit_pin, 0) ;
}

static struct file_operations fops = {
	.owner   = THIS_MODULE,
	.open    = device_open,
	.release = device_release,
	.read    = device_read,
	.write   = device_write
};

int init_module(void)
{
	Major = register_chrdev(0,DEVICE_NAME, &fops);
	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}
	if(check_mem_region(PORT, RANGE) < 0)
	{
		unregister_chrdev(Major, DEVICE_NAME);
		return -ENODEV;
	}
	else
	{
		if(request_mem_region(PORT, RANGE, DEVICE_NAME) == NULL)
		{
			unregister_chrdev(Major, DEVICE_NAME);
			return -ENODEV;
		}
		else
		{
		
		
		}
	}
	return SUCCESS;
}

void cleanup_module(void)
{
	printk(KERN_ALERT "Ade :-)\n");
	release_mem_region(PORT, RANGE); 
	unregister_chrdev(Major, DEVICE_NAME);
}