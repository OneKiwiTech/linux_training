#ifndef GPIO_H
#define GPIO_H

void gpio_init();
void gpio_set_pin_mode_ouput(int gpio_pin);
void gpio_set_pin_state(int state);
void gpio_deinit();

#endif 