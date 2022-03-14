static:
	gcc -c -Wall btm.c
	ar rcs libbtm.a btm.o
	gcc -c -Wall main.c
	gcc main.o libbtm.a -o main
	./main

shared:
	gcc -Wall -fPIC -shared btm.c -o libbtm.so
	gcc -Wall  main.c -L. -Wl,-rpath=. -lbtm -o main
	./main

clean:
	rm -f *.o *.a *.so main temp