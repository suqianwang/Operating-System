# makefile

all: dataserver client

reqchannel.o: reqchannel.H reqchannel.C
	g++ -c -g reqchannel.C

semaphore.o: semaphore.H semaphore.C
	g++ -c -g semaphore.C

dataserver: dataserver_825009505.C reqchannel.o
	g++ -g -o dataserver dataserver_825009505.C reqchannel.o -lpthread

client: client_825009505.C reqchannel.o semaphore.o boundedbuffer.H
	g++ -g -o client client_825009505.C reqchannel.o semaphore.o -lpthread

clean:
	rm -f *.o client dataserver fifo*
