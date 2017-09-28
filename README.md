# directorySearch
Recursive directory searching in C

               
/* -----------------------------------------  R E A D M E ------------------------------------------- */



NOTE: 
      - check_input() included in main.c is based on the "GetOpt Long Option Example" as
      	part of the GNU C library manual: https://www.gnu.org/software/libc/manual/html_node/Getopt-
	Long-Option-Example.html#Getopt-Long-Option-Example
      - itoa() included in main.c and my_printf.c is taken from the itoa provided in the assignment
      	page

Included in the directory are the following files:
	 - Makefile:
	   - contains shell commands for compiling and running finds & my_printf
	   
	 - main.c:
	   - contains functions for getting user input, recursively traversing through a directory,
	     and searching the files within for a user-inputted string. also times the output of
	     this operation, if specified, and records it into the "average_time" file (see bottom
	     for more details). the program will record the time if the first argument inputted is
	     'time'.  the following functions are within main.c:

	   - check_input(): uses a variation of getopt_long() to get user inputs. contains error
	     checking for incorrect input.
	     - input template: -p pathname -f flag -l -s string
	       -p: required input. also requires a followup argument. specifies the pathname to 
	       	   begin traversal.
	       -f: optional input, but requires a followup argument if included. specifies the
	       	   file extension to search through. valid arguments are: c, h, and S for ".c",
		   ".h", and ".S" files respectively. NOTE: traverse() searches for all regular files
		   if -f is not specified.
	       -l: optional input, no followup argument. specifies the inclusion of symbolic links
	           on the traversal in addition to regular files. NOTE: -f specified extension still
		   applies.
	       -s: required input. also requires a followup argument. specifies the string to search
	       	   for. may include special characters (see start_check() and end_check() for more
		   details.

	   - traverse(): starts at the specified pathname, and traverses recursively through the
	     pathname. checks each file inside the directory, and continues to check each line for
	     the correct input if the file meets the proper specifications - specs are defined in
	     the check input.

	   - start_check(): begins processing of inputted string. returns 1 on a successful check,
	     		    and 0 on a failure. cycles through line to find valid starting points
			    of the check, and uses end_check() to actually perform the check.

	   - end_check(): takes in a line and a string, and checks if the line contains the substring
	     		  includes support for the following special characters:
			  - '.': matches any character in between two other characters.
			  - '*': matches zero or more instances of the previous character.
			  - '?': matches zero or one instance of the previous character.

 	   - itoa(): converts integer to a string (used for writing output). taken from assignment1
 	    	     page.

           - check_string(): checks the user inputted string. ensures the string has no more than one
	     		     of each special character, and no special characters are directly next to
			     each other.


        - my_printf.c: functionally identical (i hope) to printf(). contains a main() function for
	  	       testing and my_printf(). supports the following format characters:
		       - '%s', '%c', '%d', '%u', '%x'
		       - includes the itoa() function outlined above
		       

        - average_time: contains timed results of running ./finds for three tests.



/* --------------------------------------------------------------------------------------------------- *\
