all: lib sys clean

lib:
	gcc -c -Wall mainlib.c
	gcc mainlib.o -o mainlib
	./mainlib ./input.txt ./output_lib.txt

sys:
	gcc -c -Wall mainsys.c
	gcc mainsys.o -o mainsys
	./mainsys ./input.txt ./output_sys.txt


clean:
	rm *.o mainsys mainlib