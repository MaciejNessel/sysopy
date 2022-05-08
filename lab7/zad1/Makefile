main:
	gcc -c -Wall main.c
	gcc main.o -o main
	gcc -c -Wall chef.c
	gcc chef.o -o chef
	gcc -c -Wall supplier.c
	gcc supplier.o -o supplier

test1: main
	./main 4 4

test2: main
	./main 10 10

clean:
	rm -f *.o
	rm -f main
	rm -f chef
	rm -f supplier