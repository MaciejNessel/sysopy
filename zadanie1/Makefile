static:
	gcc -c -Wall btm.c
	ar rcs libbtm.a btm.o

shared:
	gcc -Wall -fPIC -shared btm.c -o libbtm.so

clean:
	rm -f *.o *.a *.so