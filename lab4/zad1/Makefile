all: main ignore handler mask pending clean

main:
	gcc -c -Wall main.c
	gcc main.o -o main

pending:
	./main pending >> raport.txt

ignore:
	./main ignore >> raport.txt

handler:
	./main handler >> raport.txt

mask:
	./main mask >> raport.txt

clean:
	rm *.o main