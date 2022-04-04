all: clean main test

main:
	gcc -c -Wall catcher.c
	gcc catcher.o -o catcher
	gcc -c -Wall sender.c
	gcc sender.o -o sender
	gcc -c -Wall main.c
	gcc main.o -o main

test:
	./main 10 KILL

clean:
	rm *.o sender catcher main
