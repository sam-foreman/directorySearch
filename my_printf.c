/*
 * Sam Foreman CS410 Assignment 1
 * my_printf
 *
*/
#include <string.h>
#include <stdlib.h>
#include <limits.h>

void itoa(char *buf, int base, int d);
void my_printf(const char *format, ...);

int main(int argc, char *argv[]) 
{
    my_printf("this is a test:\n%s\t%c\t%d\t%u\t%x\n", "string", 'c', -12345, 12345, 12345);
    my_printf("this string has no extra args.\n");
    my_printf("this string has %d extra %ss.\n", 2, "arg");
    return 0;
}


void my_printf(const char *format, ... )
{
    //get arguments
    char **arg = (char**) &format;
    
    char out[10000];
    int outpoint = 0;			     
    int ac = 0;
    int i;
    //increment argument counter (to jump past format)
    arg++;
    //cycle through format argument
    for (i = 0; i < strlen(format); i++)
    {
	//if the format character isn't '%', it can be safely added to the output
	if (format[i] != '%')
	    out[outpoint++] = format[i];
	else
	{
	    //...else check the next char to see what type the variable argument is
	    i++;
	    if (format[i] == 's')
	    {
	       

		int j;
		char *newarg;
		newarg = *arg;
		for (j = 0; j < strlen(newarg); j++)
		    out[outpoint++] = newarg[j];	       

		
	    }
	    else if (format[i] == 'c')
	    {
		out[outpoint++] = (int)*arg;
	    }
	    else if (format[i] == 'd')
	    {
		char newarg[1000];
		itoa(newarg, 100,(int) *arg);
		int j;
		for (j = 0; j < strlen(newarg); j++)
		    out[outpoint++] = newarg[j];
	    }
	    else if (format[i] == 'u')
	    {
		char newarg[1000];
		unsigned int y;
		if ((int)*arg < 0)
		{
		    y = (int) *arg + (UINT_MAX + 1);
		}
		else
		    y = (int)*arg;
		itoa(newarg, 100, y);
		int j;
		for (j = 0; j < strlen(newarg); j++)
		    out[outpoint++] = newarg[j];
	    }

	    else if (format[i] == 'x')
	    {
		char newarg[1000];
		itoa(newarg, 120, (int)*arg);
		int j;
		for (j = 0; j < strlen(newarg); j++)
		    out[outpoint++] = newarg[j];
		   
	    }
	    else
		out[outpoint++] = format[i];
	    arg++;
	    
	}
    }
    //increment to get end of string
    out[outpoint++] = format[i++];
    out[outpoint++] = format[i++];

    //write to stdout
    write(1, out, strlen(out));
    return;
    
}



/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal. */
void itoa (char *buf, int base, int d)
{
    char *p = buf;
    char *p1, *p2;
    unsigned long ud = d;
    int divisor = 10;
     
    /* If %d is specified and D is minus, put `-' in the head. */
    if (base == 'd' && d < 0)
    {
	*p++ = '-';
	buf++;
	ud = -d;
    }
    else if (base == 'x')
	divisor = 16;
     
    /* Divide UD by DIVISOR until UD == 0. */
  do
  {
      int remainder = ud % divisor;
     
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
  }
  while (ud /= divisor);
     
  /* Terminate BUF. */
  *p = 0;
     
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
  {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
  }
}
