#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <ctime>

using namespace std;

int main(){
	mkdir("DRIVE", 0777);

	int y = 0;
	string name = "DRIVE/SEGMENT";
	string checkpointstr = "DRIVE/CHECKPOINT_REGION";
	string fileNameMap = "DRIVE/FILENAMEMAP";
	ofstream outfile2(checkpointstr+".txt", ios::binary);
	ofstream outfile3(fileNameMap+".txt");

	for (int i = 0; i < 64; i++){
		string iter = to_string(i);
		string file = name + iter + ".txt";
		ofstream outfile(file, ios::binary);

		if(!outfile.is_open()){
			cerr << "Problem with segment " << i << endl;
			exit(-1);
		}

		for (int x = 0; x < 1048576 / 4; x++){
			outfile.write(reinterpret_cast<const char*>(&y), sizeof(y));
		}

		outfile.close();
	}

	if(!outfile2.is_open()){
		cerr << "Checkpoint region not opened" << endl;
		exit(-1);
	}

	char buffer[1] = {0};
	for (int x = 0; x < 1048576; x++){
		outfile2.write(buffer, 1);
	}

	if(!outfile3.is_open()){
		cerr << "File name map not opened" << endl;
		exit(-1);
	}

	//10000 possible entries in filenamemap, 128 length of name.
	//? no set length of name, set 128 arbitrarily
	for(int i = 0; i < 10000 * 128; i++){
		outfile3 << '0';
	}
}
