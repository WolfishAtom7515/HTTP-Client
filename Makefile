CC=g++
CFLAGS=-I.

client: client.cpp requests.cpp helper.cpp
	$(CC) -o client client.cpp requests.cpp helper.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
