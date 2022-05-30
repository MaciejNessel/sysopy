all: test

main:
	gcc -c -Wall -lpthread  main.c
	gcc main.o -o main -lpthread

test:
	./main

clean:
	rm -f *.o
	rm -f main