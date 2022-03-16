all: static shared

static:
	gcc -c -Wall btm.c
	ar rcs libbtm.a btm.o
	gcc -c -Wall main.c
	gcc main.o libbtm.a -o main
	echo "static" | tee raport2.txt
	make tests

shared:
	gcc -Wall -fPIC -shared btm.c -o libbtm.so
	gcc -Wall  main.c -L. -Wl,-rpath=. -lbtm -o main
	echo "shared" | tee -a raport2.txt
	make tests

clean:
	rm -f *.o *.a *.so main

tests:
	./main create_table 10 \
	wc_files ./1.txt \
	wc_files ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt \
	wc_files ./2.txt \
	wc_files ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt \
	wc_files ./3.txt \
	wc_files ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt \
	remove_block 0 remove_block 1 remove_block 2 remove_block 3 remove_block 4 remove_block 5
	./main create_table 10 \
	wc_files ./1.txt remove_block 0 wc_files ./1.txt remove_block 1 wc_files ./1.txt remove_block 2 wc_files ./1.txt remove_block 3

