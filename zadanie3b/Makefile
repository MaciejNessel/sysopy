COMMAND1 =   create_table 10 \
            wc_files ./1.txt \
            wc_files ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt ./1.txt \
            wc_files ./2.txt \
            wc_files ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt ./2.txt \
            wc_files ./3.txt \
            wc_files ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt ./3.txt \
            remove_block 0 remove_block 1 remove_block 2 remove_block 3 remove_block 4 remove_block 5
COMMAND2 = create_table 10 \
            wc_files ./1.txt remove_block 0 wc_files ./1.txt remove_block 1 wc_files ./1.txt remove_block 2 wc_files ./1.txt remove_block 3

opt_lvl = O2

all: static shared dynamically_loaded

static:
	make clean
	gcc -c -Wall -$(opt_lvl) btm.c
	ar rcs libbtm.a btm.o
	gcc -c -Wall -$(opt_lvl) main.c
	gcc main.o libbtm.a -o main
	echo "static opt_lvl: " $(opt_lvl) | tee -a raport3b.txt
	./main $(COMMAND1)
	./main $(COMMAND2)

shared:
	make clean
	gcc -Wall -$(opt_lvl) -fPIC -shared btm.c -o libbtm.so
	gcc -Wall -$(opt_lvl) main.c -L. -Wl,-rpath=. -lbtm -o main
	echo "shared opt_lvl: " $(opt_lvl) | tee -a raport3b.txt
	./main $(COMMAND1)
	./main $(COMMAND2)


dynamically_loaded:
	make clean
	gcc -Wall -$(opt_lvl) -fPIC -shared btm.c -o libbtm.so
	gcc -Wall -$(opt_lvl) main_dynamic.c -ldl -o main
	echo "dynamically_loaded opt_lvl: " $(opt_lvl) | tee -a raport3b.txt
	./main $(COMMAND1)
	./main $(COMMAND2)

clean:
	rm -f *.o *.a *.so main

