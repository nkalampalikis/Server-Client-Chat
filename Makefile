

all:
	$(CC) -Wall server.c -O2 -lpthread -o server

clean:
	$(RM) -rf server
