all: main.cpp inode.h
	g++ main.cpp -g -o ./main -std=c++14

clean:
	rm -rf main
	rm -r DRIVE
