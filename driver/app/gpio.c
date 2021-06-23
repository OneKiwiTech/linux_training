#include "config.h"

#if GPIO_USE_LIB_BCM35
#include <bcm2835.h>
#endif

#ifdef GPIO_USE_LIB_WIRING_PI
#include <wiringPi.h>
#endif

void gpio_init()
{
#ifdef GPIO_USE_LIB_WIRING_PI
    wiringPiSetup ();
#endif

#ifdef GPIO_USE_LIB_BCM35
    bcm2835_init();
#endif    
}

void gpio_set_pin_mode_ouput(int gpio_pin)
{
#ifdef GPIO_USE_LIB_WIRING_PI
      pinMode (gpio_pin, OUTPUT) ; 
#endif 

#ifdef GPIO_USE_LIB_BCM35
    bcm2835_gpio_fsel(gpio_pin, BCM2835_GPIO_FSEL_OUTP);
#endif    
}

void gpio_set_pin_state(int gpio_pin, int state)
{
#ifdef GPIO_USE_LIB_WIRING_PI
    digitalWrite (gpio_pin, !state) ; 
#endif
#ifdef GPIO_USE_LIB_BCM35
    bcm2835_gpio_write(gpio_pin, !state);
#endif 
}

void gpio_deinit()
{
#ifdef GPIO_USE_LIB_BCM35
    bcm2835_close();
#endif
}