# makefile

all: dataserver simpleclient

reqchannel.o: reqchannel.H reqchannel.C
	g++ -c -g reqchannel.C

semaphore.o: semaphore.H semaphore.C
	g++ -c -g semaphore.C

dataserver: dataserver.C reqchannel.o 
	g++ -g -o dataserver dataserver.C reqchannel.o -lpthread

simpleclient: simpleclient.C reqchannel.o semaphore.o boundedbuffer.H
	g++ -g -o simpleclient simpleclient.C reqchannel.o semaphore.o -lpthread

clean:
	rm -f *.o simpleclient dataserver
