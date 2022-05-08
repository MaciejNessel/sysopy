main:
	gcc -c -Wall main.c
	gcc main.o -o main -lrt -lpthread
	gcc -c -Wall chef.c
	gcc chef.o -o chef -lrt -lpthread
	gcc -c -Wall supplier.c
	gcc supplier.o -o supplier -lrt -lpthread

test1: main
	./main 4 4

test2: main
	./main 10 10

clean:
	rm -f *.o
	rm -f main
	rm -f chef
	rm -f supplier
