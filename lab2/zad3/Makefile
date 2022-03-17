all: first second clean

first:
	gcc -c -Wall main_first.c
	gcc main_first.o -o main_first
	./main_first /sys/module

second:
	gcc -c -Wall main_second.c
	gcc main_second.o -o main_second
	./main_second /sys/module

clean:
	rm *.o main_first main_second