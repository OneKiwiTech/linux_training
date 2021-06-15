#undef	PHOTO_HACK

#include <wiringPi.h>

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

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

static int segments [7] = {  6,  5,  4,  3,  2,  1, 0 } ;


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
//	read from by the thread that updates the display. Only the first
//	6 characters are used.

char display;


/*
 * displayDigits:
 *	This is our thread that's run concurrently with the main program.
 *	Essentially sit in a loop, parsing and displaying the data held in
 *	the "display" global.
 *********************************************************************************
 */

PI_THREAD (displayDigits)
{
  int digit, segment ;
  int index, d, segVal ;

  piHiPri (50) ;

  for (;;)
  {
      for (segment = 0 ; segment < 7 ; ++segment)
      {
		d = toupper (display) ;
		/**/ if ((d >= '0') && (d <= '9'))	// Digit
		  index = d - '0' ;
		else if ((d >= 'A') && (d <= 'F'))	// Hex
		  index = d - 'A' + 10 ;
		else
		  index = 16 ;				// Blank

		segVal = segmentDigits [index * 7 + segment] ;

		digitalWrite (segments [segment], segVal) ;
      }
      digitalWrite (digits, 1) ;
      delay (2) ;
      digitalWrite (digits, 0) ;
  }
}


void hw_init (void)
{
  int i, c ;

  gpio_init();

  // 7 segments
  for (i = 0 ; i < 7 ; ++i)
  { 
      pinMode (segments [i], OUTPUT) ; 
      digitalWrite (segments [i], 0) ; 
        
  }

  // 6 digits
  digitalWrite (digits , 0) ;   
  pinMode (digits ,   OUTPUT); 

  piThreadCreate (displayDigits) ;
  delay (10) ; // Just to make sure it's started
}

int main (void)
{
  struct tm *t ;
  time_t     tim ;

  setup    () ;

  tim = time (NULL) ;
  for (;;)
  {
    while (time (NULL) == tim)
      delay (5) ;

    tim = time (NULL) ;
    t   = localtime (&tim) ;

    t->tm_sec;

    delay (500) ;
  }

  return 0 ;
}