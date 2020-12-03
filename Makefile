#Makefile de la Fita 4 pel codi que governa els sensors d'orientació.
#Implementat l'ús de timers i de llibreries dinàmiques.

CC = g++
OBJECTS = func.o libfunc.so

all: $(OBJECTS) orientacio.o

func.o:
	$(CC) -fPIC -c -o func.o func.c

libfunc.so:
	$(CC) -shared -fPIC -o libfunc.so func.o

orientacio.o:
	$(CC) -c -o orientacio.o orientacio.cpp -l.
	$(CC) -o orientacio orientacio.o -lsqlite3 -lRTIMULib -lrt -lpthread  -L. -lfunc
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/pi/Desktop/GIT/SensorsSenseHat
	./orientacio -s iotlab.euss.cat -d /home/pi/Desktop/GIT/SensorsSenseHat/basedades_adstr.db

clean:
	rm *.o
