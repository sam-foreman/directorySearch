all: finds my_printf

my_printf: my_printf.c
	gcc -m32 -o my_printf my_printf.c

finds: main.c
	gcc -o finds main.c

clean:
	rm -f *.o *.~ *.# *~ *# finds my_printf
