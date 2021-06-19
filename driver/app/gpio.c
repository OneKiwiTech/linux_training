#include <bcm2835.h>

#define GPIO_USE_LIB_WIRING_PI
// #define GPIO_USE_LIB_BCM35

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
    bcm2835_gpio_fsel(gpio_pin, BCM2835_GPIO_FSEL_OUTP);
#endif 

#ifdef GPIO_USE_LIB_BCM35
    pinMode (gpio_pin, OUTPUT) ; 
#endif    
}

void gpio_set_pin_state(int gpio_pin, int state)
{
#ifdef GPIO_USE_LIB_WIRING_PI
    bcm2835_gpio_write(gpio_pin, state);
#endif
#ifdef GPIO_USE_LIB_BCM35
    pinDigitalWrite (gpio_pin, state) ; 
#endif 
}

void gpio_deinit()
{
#ifdef GPIO_USE_LIB_BCM35
    bcm2835_close();
#endif
}