all: main.cpp inode.h
	g++ main.cpp -g -o ./main -std=c++14

clean:
	rm -rf main
	rm -rf test*.txt
	rm -rf a.txt
	rm -rf outFile.txt
	rm -r DRIVE
