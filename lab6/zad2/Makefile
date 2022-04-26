all:  client server

client:
	gcc -c -Wall client.c
	gcc client.o -o client -lrt

server:
	gcc -c -Wall server.c
	gcc server.o -o server -lrt

clean:
	rm server client *.o