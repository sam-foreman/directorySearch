#include <stdio.h> 
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>        
#include <getopt.h>
#include <stdlib.h>                                                            
#include <ctype.h>
#define rdtsc(x) __asm__ __volatile__("rdtsc \n\t" : "=A" (*(x)))


void itoa (char *buf, int base, int d); 
int main(int argc, char *argv[])
{
    
    int cantime = 0;
    //allow the program to write the time to "average_time" if the first argument is "time"
    if (!(strcmp(argv[1], "time")))
   	cantime++;	 
    unsigned long long start, finish;
    rdtsc(&start);

    //check argv[] for pathname / filetype etc.
    int i = check_input(argc, argv);
    if (i != 0)
	exit(0);
    rdtsc(&finish);

    //write the results to a file (if cantime is specified)
    if (cantime)
    {
	int statsFile;
	char stats[100];
	unsigned long time = (finish - start) / 2600;
	itoa(stats, 'd', time);
	strcat(stats, "\n");
	statsFile = open("average_time", O_WRONLY | O_CREAT | O_APPEND, 0666);
	write(statsFile, stats, strlen(stats));
	close(statsFile);
    }

    return 0;
}

int check_input(int argc, char **argv)
{
/* takes the arguments directly from main() for processing
 * after going through the arguments, calls traverse to walk through the directories
 */
    static int verbose_flag;
    int c;
    char *pathArg = NULL;
    char *typeArg = NULL;
    char *stringArg = NULL;
    char linkArg = 0;
    
    while (1)
    {

	//specifies the 4 options to be used
	static struct option long_options[] =
	{
	    {"verbose", no_argument, &verbose_flag, 1},
	    {"brief", no_argument, &verbose_flag, 0},
	    
	    {"path", required_argument, 0, 'p'},
	    {"type", required_argument, 0, 'f'},
	    {"link", no_argument, 0, 'l'},
	    {"string", required_argument, 0, 's'},
	    {0, 0, 0, 0}
	};
	//getopt_long stores option index here
	
	int option_index = 0;
	// -p and -s are both required, -p, -s, and -f all require a followup argument
	c = getopt_long (argc, argv, "p:f:ls:", long_options, &option_index);
	
	//detect last option
	if (c == -1)
	    break;
	switch (c)
	{
	case 0:
	    //if this option set a flag, do nothing else
	    if (long_options[option_index].flag != 0)
		break;
	    printf("option %s", long_options[option_index].name);
	    if (optarg)
		printf(" with arg %s", optarg);
	    printf("\n");
	    break;

	case 'p':
	    pathArg = optarg;
	    //printf("option -p with arg '%s'\n", optarg);
	    break;

	case 'f':
	    typeArg = optarg;
	    //printf("option -f with value '%s'\n", optarg);
	    break;

	case 'l':
	    linkArg = 1;
	    //puts("option -l");
	    break;

	case 's':
	    stringArg = optarg;
	    //printf("option -s with value '%s'\n", optarg);
	    break;

	case '?':
	    //in this case, getopt_long would have already printed an error
	    break;

	default:
	    return -1;
	}
    }
/*  Instead of reporting '--verbose' and '--brief' as they are encountered,
    we report the final status resulting from them */
    if (verbose_flag)
	printf("verbose flag is set");
    
    //prints any remaining command line arguments
    if (optind < argc)
    {
	printf("non-option ARGV-elements: ");
	while (optind < argc)
	    printf("%s ", argv[optind++]);
	putchar ('\n');
    }

    //check to make sure -p, and -s (the required options) were specified
    if (pathArg == NULL || stringArg == NULL)
    {
	printf("error. missing required arguments.\n");
	return -1;
    }

    //check to make sure stringArg is valid 
    if (check_string(stringArg) == 0)
	return 0;

    //call the directory traversal
    traverse(pathArg, stringArg, linkArg, typeArg);
    return 0;
}

int traverse(char *pathname, char const *stringArg, char const *linkArg, char const *typeArg)
{
/* traverse takes in the options specified in check_input and walks recursively along the pathname
 * it checks each file along the way if it contains stringArg
 *
 */

    //setup structures for directories
    struct stat stat;
    DIR *dir;
    struct dirent *dp;
    FILE *file;
    char fullname[FILENAME_MAX];
    int len = strlen(pathname);
    if (len >= FILENAME_MAX - 1)
    {
	printf("error. len > max filename\n");
	return -1;
    }

    strcpy(fullname, pathname);

    //append '/' to pathname
    fullname[len++] = '/';

    if (!(dir = opendir(pathname)))
    {
	printf("can't open %s\n", pathname);
	return -1;
    }

    //open directory, and cycle through its contents
    while ((dp = readdir(dir)))
    {
	//ignore if current or parent directory (this avoids an infinite loop)
	if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
	    continue;

	//get new filename, and lstat it (so we can check what kind of file it is)
	strncpy(fullname+len, dp->d_name, FILENAME_MAX-len);
	if (lstat(fullname, &stat) == -1)
	{
	    printf("error. can't stat %s\n", fullname);
	    continue;
	}

	//if the file is a symbolic link
	if (S_ISLNK(stat.st_mode))
	{

	    //if symbolic links are not specified (no -l), ignore the file
	    if (linkArg == NULL)
		continue;

	    // this is all repeated code...I should probably make this a separate function
	    if (typeArg != NULL)
	    {
		char linkBuf[1000];
		readlink(fullname, linkBuf, sizeof(stat)-1);
		if (!(strcmp(typeArg, "c")) && strstr(linkBuf, ".c") == NULL)
		    continue;
		else if (!(strcmp(typeArg, "h")) && strstr(fullname, ".h") == NULL)
		    continue;
		else if (!(strcmp(typeArg, "S")) && strstr(fullname, ".S") == NULL)
		    continue;
	    }
	    file = fopen(fullname, "r");
	    if (file != NULL)
	    {
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		int didread = 0;
		while ((read = getline(&line, &len, file)) != -1)
		{
		    if (strstr(line, stringArg) != NULL || start_check(line, stringArg) == 1)
		    {
			if (didread++ == 0)
			{
			    char pb[PATH_MAX];
			    
			    realpath(fullname, pb);
			    printf("%s:\n", pb);
			}
			printf("%s", line);
		    }
		}
		if (didread == 1)
		{
		    printf("\n");
		    didread--;
		}
		fclose(file);
	    }
	    continue;
	}

	//if file is a regular file
	if (S_ISREG(stat.st_mode))
	{ 

	    //if type are is specified, that means DON'T search through all files.
	    if (typeArg != NULL)
	    {
		if (!(strcmp(typeArg, "c")) && strstr(fullname, ".c") == NULL)
		    continue;
		else if (!(strcmp(typeArg, "h")) && strstr(fullname, ".h") == NULL)
		    continue;
		else if (!(strcmp(typeArg, "S")) && strstr(fullname, ".S") == NULL)
		    continue;

	    }

	    //open file and read each line
	    file = fopen(fullname, "r");
	    if (file != NULL)
	    {
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		int didread = 0;
		while ((read = getline(&line, &len, file)) != -1)
		{
		    //if the line contains the string, print it
		    if (strstr(line, stringArg) != NULL || start_check(line, stringArg) == 1)
		    {				

			//...but first print the filename if this is it's first line found
			if (didread++ == 0)
			{
			    char pb[PATH_MAX];
			    realpath(fullname, pb);
			    printf("%s:\n", pb);
			}
			printf("%s", line);
		    }
		}
		if (didread == 1)
		{
		    printf("\n");
		    didread--;
		}
		fclose(file);
	    }
	    continue;
	}

	//this just checks for all the other filetypes. 
	if (S_ISBLK(stat.st_mode))
	{
	    printf("%s is a block file.\n", fullname);
	    continue;
	}

	if (S_ISCHR(stat.st_mode))
	{
	    printf("%s is a character file.\n", fullname);
	    continue;
	}

	if (S_ISFIFO(stat.st_mode))
	{
	    printf("%s is a pipe or FIFO.\n", fullname);
	    continue;
	}

	if (S_ISSOCK(stat.st_mode))
	{
	    printf("%s is a socket.\n", fullname);
	    continue;
	}

	//if it's a directory, make the recursive call on said directory
	if (S_ISDIR(stat.st_mode))
	    traverse(fullname, stringArg, linkArg, typeArg);
    }
    //finally, close the directory
    if (dir) closedir(dir);
}



int start_check(char * line, char * string)
{
/* start_check is made to make the stringArg checker a little easier
 * essentially it finds potential starting places where the string could match and THEN performs the check (end_checK())
 * 
 */
    if (strstr(string, line) != NULL)
	return 1;
    else
    { 
	//if the string is only a special character, return true. logically this makes sense since '.' for example can represent
	//any character
	if (strlen(string) == 1 && (string[0] == '*' || string[0] == '?' || string[0] == '.'))
	    return 1;
	else if (string[0] == '*' || string[0] == '?' || string[0] == '.')
	    string++;
	if (string[strlen(string)-1] == '.')
	{
	    string[strlen(string)-1] = 0;
	}
	int i;
	//cycle through the line
	for (i = 0; i < strlen(line); i++)
	{
	    //if line[i] == first char of the string, begin the real check
	    if (line[i] == string[0])
	    {
		//if the strings match, return 1. note that there may be more oportunities to check
		//in other words, don't return false if they don't match
		if (end_check(line + i, string) == 1)
		    return 1;
	    }
	}
	//ok now return false, there's no more line to check...
	return 0;
    }
}


int end_check(char * line, char * string)
{
/* end_check takes in a line and a string, and determines if they match, taking into account for special characters
 *
 *
 */
    //the string and the line have separate counters, if one reaches the end, the check is over
    int sPos = 0;
    int lPos = 0;
    while (1)
    {
	if (lPos >= strlen(line))
	    return 0;
	if (sPos >= strlen(string))
	    return 1;
	
	//if the characters match, keep going
	if (line[lPos] == string[sPos])
	{
	    lPos++;
	    sPos++;
	    continue;
	}
	
	//if not, check for special characters
	else if (string[sPos] == '.')
	{
	    lPos++;
	    sPos++;
	    continue;
	}
	else if (string[sPos] == '*')
	{
	    //the line and string match indefinitely as long as the character is the same
	    while (line[lPos] == string[sPos-1])
		lPos++;
	    sPos++;
	    /**/ continue;
	}
	else if (string[sPos] == '?')
	{
	    //the line and string can only match one extra time (hence the if, not the while)
	    if (line[lPos] == string[sPos-1])
		lPos++;
	    sPos++;
	    /**/continue;
	}
	else if (string[sPos+1] == '?' || string[sPos+1] == '*')
	{
	    sPos++;
	    /**/ continue;
	}

	//if the characters don't match, and the string isn't a special character, fail.
	else
	    return 0;
		       
    }
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

int check_string(char * string)
{
/* check_string takes the inputted stringArg and makes sure it is valid
 * invalid strings include non-alphanumeric characters that don't include '.', '*', or '?'
 * also invalid is more than one special character, or two special characters next to each other
 */
    int i;
    //count number of special chars. if these reach > 1, return false
    int period = 0;
    int star = 0;
    int question = 0;
    //cycle through string and check each character
    for (i = 0; i < strlen(string); i++)
    {
	if (isalpha(string[i]) || isdigit(string[i]))
	    continue;
	else if (string[i] == '.')
	{
	    if (++period > 1)
	    {
		printf("error. only one special character '.' allowed.\n");
		return 0;
	    }
//	    if (string[i+1] == '*' || string[i+1] == '?')
//	    {
//		printf("error. special characters must be separated by at least one regular character.\n");
//	    }
	}
	else if (string[i] == '*')
	{
	    if (++star > 1)
	    {
		printf("error. only one special character '*' allowed.\n");
		return 0;
	    }
//	    if (string[i+1] == '.' || string[i+1] == '?')
//	    {
//		printf("error. special characters must be separated by at least one regular character.\n");
//		return 0;
//	    }
	}
	else if (string[i] == '?')
	{
	    if (++question > 1)
	    {
		printf("error. only one special character '?' allowed.\n");
		return 0;
	    }
//	    if (string[i+1] == '.' || string[i+1] == '*')
//	    {
//		printf("error. special characters must be separated by at least one regular character.\n");
//		return 0;
//	    }
	}
	else
	{
	    printf("error. %c is an illegal character.\n", string[i]);
	    return 0;
	}
    }
    return 1;
}
