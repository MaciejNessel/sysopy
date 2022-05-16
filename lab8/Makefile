all: main tests-block tests-numbers

main:
	gcc -c -Wall main.c
	gcc main.o -o main -lpthread

tests-block:
	for no_threads in 1 2 4 8 16 ; do \
		echo "------Test block------" >> Times.txt; \
		./main $$no_threads block ./mountain.ascii.pgm ./results/$$no_threads-block-output-mountain.ascii.pgm >> Times.txt ;\
		echo " ">> Times.txt ;\
	done

tests-numbers:
	for no_threads in 1 2 4 8 16 ; do \
		echo "------Test numbers------" >> Times.txt ; \
		./main $$no_threads numbers ./mountain.ascii.pgm ./results/$$no_threads-numbers-output-mountain.ascii.pgm >> Times.txt; \
		echo " ">> Times.txt ;\
	done

clean:
	rm -f *.o
	rm -f main

