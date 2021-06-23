#include "gpio.h"
#include "config.h"

#ifdef GPIO_USE_LIB_WIRING_PI
  #include <wiringPi.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

const char* test_string = "0123456789ABCDEF";

/*
 *  Segment mapping
 *
 *	 --a--
 *	|     |
 *	f     b
 *	|     |
 *	 --g--
 *	|     |
 *	e     c
 *	|     |
 *	 --d--  p
 */

// GPIO Pin Mapping
// https://pinout.xyz/pinout/wiringpi

#ifdef GPIO_USE_LIB_WIRING_PI
 static int segments [7] = {  4,  17,  27,  22, 5,  6, 26 } ;
#endif


// https://forums.ni.com/t5/Community-Documents/LabVIEW-BCM2835-Library-for-Raspberry-Pi/ta-p/3539080?profile.language=en
#ifdef GPIO_USE_LIB_BCM35
 static int segments [7] = {  4,  17,  27,  22, 5,  6, 26 } ;
#endif

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
 

// display:
//	A global variable which is written to by the main program and
//	read from by the thread that updates the display.
static char display = 0;

void displayDigits(char ch)
{
  int segment ;
  int index, d, segVal ;

  for (segment = 0 ; segment < 7 ; ++segment)
  {
    d = toupper (ch) ;
    if ((d >= '0') && (d <= '9'))	// Digit
    {
      index = d - '0' ;  // 1 <=> 0x31 (ASCII) = 0x31 - 0x30 ('0')
    }else if ((d >= 'A') && (d <= 'F'))	// Hex
    {
      index = d - 'A' + 10 ;
    }else
    {
      index = 16 ;				// Blank
    }

    segVal = segmentDigits [index * 7 + segment] ;
    // printf("segment val = %d\n", segVal);
    gpio_set_pin_state (segments [segment], segVal) ;
  }
}


void hw_gpio_init (void)
{
  int i = 0;

  gpio_init();

  // 7 segments
  for (i = 0 ; i < 7 ; ++i)
  { 
      gpio_set_pin_mode_ouput(segments [i]);
      gpio_set_pin_state (segments [i], 0) ; 
  }
}

int main (void)
{
  char i = 0;
  char* str_ptr = NULL;
  char c = 0;
  // Init gpio state
  hw_gpio_init() ;

  for (;;)
  {
    str_ptr = &test_string[0];
    while(*str_ptr != NULL)
    {
      c = *str_ptr++;
      printf("%c\n", c);
      displayDigits(c);
      delay (1000) ;
    }
  }

  return 0 ;
}