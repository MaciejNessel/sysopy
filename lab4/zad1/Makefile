all: main ignore handler mask pending clean

main:
	gcc -c -Wall main.c
	gcc main.o -o main

pending:
	./main pending 2>&1 | tee -a raport.txt

ignore:
	./main ignore 2>&1 | tee -a raport.txt

handler:
	./main handler 2>&1 | tee -a raport.txt

mask:
	./main mask 2>&1 | tee -a raport.txt

clean:
	rm *.o main