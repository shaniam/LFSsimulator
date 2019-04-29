all: main.cpp
	g++ main.cpp -o ./main
clean:
	rm -rf *.txt
	rm -rf main
	rm -r DRIVE
