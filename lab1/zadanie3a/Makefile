
all: static shared dynamically_loaded clean

static:
	make clean
	gcc -c -Wall btm.c
	ar rcs libbtm.a btm.o
	gcc -c -Wall main.c
	gcc main.o libbtm.a -o main
	echo "static" | tee raport3a.txt
	make tests


shared:
	make clean
	gcc -Wall -fPIC -shared btm.c -o libbtm.so
	gcc -Wall  main.c -L. -Wl,-rpath=. -lbtm -o main
	echo "shared" | tee -a raport3a.txt
	make tests


dynamically_loaded:
	make clean
	gcc -Wall -fPIC -shared btm.c -o libbtm.so
	gcc -Wall main_dynamic.c -ldl -o main
	echo "dynamically_loaded" | tee -a raport3a.txt
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

