A: drives

all: main.cpp inode.h
	g++ main.cpp -g -o ./main -std=c++14
	
A.o: drives.cpp
	g++ -c drives.cpp -g -std=c++14
	
A: drives.o
	g++ -g drives.o -o drives -std=c++14
	
drive: drives
	./drives

clean:
	rm -rf main
	rm -r DRIVE
