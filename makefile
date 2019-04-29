all: main.cpp
	g++ main.cpp -g -o ./main
clean:
	rm -rf main
	rm -r DRIVE
