all: main test clean

main:
	gcc -c -Wall main.c
	gcc main.o -o main

test:
	./main input.txt

clean:
	rm *.o main