binaries = check
objects  = src/check.o

check : $(objects)
	gcc -o check $(objects)
	
check.o : src/check.c	   # module for twiddling the frobs cc -c -g foo.c
	gcc -Wall -c src/check.c
	

src/*.c :

clean: FORCE 
	rm $(binaries)
	rm $(objects) 
FORCE:
