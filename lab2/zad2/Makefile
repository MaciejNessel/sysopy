all: lib sys clean

lib:
	gcc -c -Wall mainlib.c
	gcc mainlib.o -o mainlib
	./mainlib a ./input.txt

sys:
	gcc -c -Wall mainsys.c
	gcc mainsys.o -o mainsys
	./mainsys a ./input.txt


clean:
	rm *.o mainlib mainsys