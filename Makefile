all: orientacio.cpp
	g++ -lRTIMULib -lsqlite3 orientacio.cpp   -o orientacio

clean:
	rm orientacio
