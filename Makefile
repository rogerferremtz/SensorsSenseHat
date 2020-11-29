all:
	g++ -lRTIMULib -lsqlite3 -lrt -lpthread orientacio.cpp   -o orientacio

clean:
	rm orientacio

