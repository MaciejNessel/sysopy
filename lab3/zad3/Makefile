all: main clean

main:
	gcc -c -Wall main.c
	gcc main.o -o main
	./main ./test "Sysopy" 3

clean:
	rm *.o main