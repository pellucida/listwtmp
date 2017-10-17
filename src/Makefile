

all: listwtmp dumpwtmp

listwtmp:	listwtmp.o
	$(CC) -o listwtmp listwtmp.o

dumpwtmp:	dumpwtmp.o
	$(CC) -o dumpwtmp dumpwtmp.o


clean: 
	rm -rf listwtmp listwtmp.o
	rm -rf dumpwtmp dumpwtmp.o

