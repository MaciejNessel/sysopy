all: test

main:
	gcc -Wall  main.c -o main
	gcc -Wall  consumer.c -o consumer
	gcc -Wall  producer.c -o producer

test: main
	./main

clean:
	rm -f main consumer producer pipe