all:  client server

client:
	gcc -c -Wall client.c
	gcc client.o -o client

server:
	gcc -c -Wall server.c
	gcc server.o -o server

clean:
	rm server client *.o