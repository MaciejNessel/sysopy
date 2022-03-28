all: main test1 test2 test3 test4 test5 clean

main:
	gcc -c -Wall main.c
	gcc main.o -o main -lm

test1:
	make clean_temp
	echo "h = 0.1 N = 10" | tee -a pomiary.txt
	(time -f 'real: %E user: %U sys: %S' ./main 0.1 10) 2>> pomiary.txt

test2:
	make clean_temp
	echo "h = 0.01 N = 10"| tee -a pomiary.txt
	time -f 'real: %E user: %U sys: %S' ./main 0.01 10 2>> pomiary.txt

test3:
	make clean_temp
	echo "h = 0.001 N = 50"| tee -a pomiary.txt
	time -f 'real: %E user: %U sys: %S' ./main 0.001 50 2>> pomiary.txt

test4:
	make clean_temp
	echo "h = 0.0001 N = 50" | tee -a pomiary.txt
	time -f 'real: %E user: %U sys: %S' ./main 0.0001 50 2>> pomiary.txt

test5:
	make clean_temp
	echo "h = 0.0001 N = 1000" | tee -a pomiary.txt
	time -f 'real: %E user: %U sys: %S' ./main 0.0001 1000 2>> pomiary.txt

clean:
	rm *.o main

clean_temp:
	mkdir -p "temp"
	rm -f ./temp/*
