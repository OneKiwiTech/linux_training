#include <bcm2835.h>

// Blinks on RPi Plug P1 pin 11 (which is GPIO pin 17)
#define PIN RPI_GPIO_P1_11

void gpio_init()
{
    wiringPiSetup ();
    bcm2835_init();
}

void gpio_set_pin_mode()
{
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
}

void gpio_set_pin_state()
{
    bcm2835_gpio_write(PIN, HIGH);
}

void gpio_deinit()
{
    bcm2835_close();
}