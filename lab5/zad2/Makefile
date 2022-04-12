all: main

main:
	gcc -c -Wall main.c
	gcc main.o -o main

send:
	./main mail@domena.pl tytul tresc

read:
	./main data

clean:
	rm *.o main