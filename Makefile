# Makefile. dont forget the tabs

options= -pthread

all: client server
client:
	gcc $(options) client.c -o client.x
server:
	gcc $(options) server.c -o server.x
run_client:
	./client.x 192.168.1.13 1234
run_server:
	./server.x 1234

clean:
	rm -rf *.o
	rm -rf *.x
