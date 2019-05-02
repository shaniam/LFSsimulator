#include <cstdlib>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include "inode.h"
#define KILO 1024

using namespace std;

int inodes=10*KILO;
int mapindex=0;
//vector<int> imap(inodes,-1);
vector<char>inMem(KILO*KILO, '0');
int file=0;
int inmem_index=0;
int segindex=KILO;
vector<int> ssbnodes={};

char openBlockInSegment[KILO * KILO];
int openBlock = 0;
int segNum = 0;
int summary[KILO][2];
int checkpoint[40];
int imap[40 * KILO];

void hardDrive(){
	mkdir("DRIVE", 0777);

	int y = 0;
	string name = "DRIVE/SEGMENT";
	string checkpoint = "DRIVE/CHECKPOINT_REGION";
	string fileNameMap = "DRIVE/FILENAMEMAP";
	ofstream outfile2(checkpoint+".txt", ios::binary);
	ofstream outfile3(fileNameMap+".txt");

	for (int i = 0; i < 64; i++){
		string iter=to_string(i);
		string file=name+iter+".txt";
		ofstream outfile(file, ios::binary);
		char y = 0;

		if(!outfile.is_open()){
			cerr << "Problem with segment " << i << endl;
			exit(-1);
		}

		//1048576 / 4
		for (int x = 0; x < 1048576; x++){
			outfile.write(reinterpret_cast<const char*>(&y), sizeof(y));
		}

		outfile.close();
	}

	if(!outfile2.is_open()){
		cerr << "Checkpoint region not opened" << endl;
		exit(-1);
	}

	for (int x = 0; x < 1048576 / 4; x++){
		outfile2.write(reinterpret_cast<const char*>(&y), sizeof(y));
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

void import(string file, string lfsFile){
	fstream fileIn(file);
  if (!fileIn.is_open()){
    cout << "File not found!" << endl;
    exit(-1);
  }

	//Determine size of file
  fileIn.seekg(0, ios::end);
  int fileSize = fileIn.tellg();
  fileIn.seekg(0, ios::beg);

	if ((fileSize / KILO) > KILO - openBlock){
		fstream seg("DRIVE/SEGMENT"+std::to_string(segNum)+".txt", ios::binary);

		seg.write(openBlockInSegment, KILO * KILO);
		seg.write(reinterpret_cast<const char*>(&summary), 8192);

		seg.close();

		openBlock = 0;
}

	//cout << "File size: " << fileSize << endl;

	fstream fileNameMap("DRIVE/FILENAMEMAP.txt");
	char temp[128];
	int iNodeNum = -1;
	int frag = iNodeNum / (KILO / 4);

	//Find an open spot in the file, fileNameMap was initialized to all 0's so anything that's not a 0 is used
  for (int i = 0; i < 10000; i++){
    fileNameMap.seekg(i * 128);
		//fill(begin(temp), end(temp), '0');
    fileNameMap.read(temp, 128);

		//string str(temp);
		//cout << "Str: " << temp << endl;

		//Found an empty spot in the fileNameMap
		if (temp[i*128] == '0'){
      iNodeNum = i;
			//cout << i << endl;
			break;
		}
	}

	//string str(temp);
	//cout << "Str: " << iNodeNum << endl;

	//every 128th bit is set in the file, then the disk must be full
	if(iNodeNum == -1){
		cerr << "Hard Drive Full!" << endl;
		exit(-1);
	}

	//Convert to C string so it can be used with .write
	int n = lfsFile.length();
  char lfsCArray[n + 1];
  strcpy(lfsCArray, lfsFile.c_str());

	//Find proper spot in file (128byte blocks), write the name, and then fill remainder of block with garbage
	fileNameMap.seekp(iNodeNum * 128);
	fileNameMap.write(lfsCArray, (n + (128 - n)));

	char inputFileBuffer[fileSize];
	fileIn.read(inputFileBuffer, fileSize);
	memcpy(&openBlockInSegment[openBlock * KILO], inputFileBuffer, fileSize);

	// for(int i = 0; i < inputFileBuffer.size(); i++){
	// 	cout << inputFileBuffer[i];
	// }

	// for(int i = 0; i < openBlockInSegment.size(); i++){
	// 	cout << openBlockInSegment[i];
	// }

	iNode inode;
	inode.fileName = lfsFile;
	inode.size = fileSize;
	for (int i = 0; i <= fileSize / KILO; i++){
    inode.dataBlock[i] = openBlock + (segNum - 1) * KILO;
    summary[openBlock][0] = iNodeNum;
    summary[openBlock][1] = i;
    openBlock++;
		//cout << i << endl;
  }

	//cout << inode.fileName << " " << inode.size << " " << inode.dataBlock.data() << endl;
	//cout << iNodeNum << endl;

	// for(int i = 0; i < inode.dataBlock.size(); i++){
	// 	cout << inode.dataBlock[i];
	// }

	summary[openBlock][0] = iNodeNum;
	summary[openBlock][1] = -1;

	memcpy(&openBlockInSegment[openBlock * KILO], &inode, sizeof(inode));
	openBlock++;

	if (openBlock == KILO){
		fstream seg("DRIVE/SEGMENT"+to_string(segNum)+".txt", ios::binary);

		seg.write(openBlockInSegment, KILO * KILO);
		seg.write(reinterpret_cast<const char*>(&summary), 8192);

		seg.close();
		cout << "run" << endl;
	}

  imap[iNodeNum] = (openBlock - 1) + (segNum * KILO);

  memcpy(&openBlockInSegment[openBlock * KILO], &imap[frag * (KILO / 4)], KILO);

  summary[openBlock][0] = -1;
  summary[openBlock][1] = frag;

  checkpoint[frag] = openBlock + segNum * KILO;

  openBlock++;

	// fstream in("DRIVE/SEGMENT0.txt", fstream::binary);
	//
	// char buffer[fileSize];
	// in.read(buffer, fileSize);
	//
	// for(int i = 0; i < fileSize; i++){
	// 	cout << buffer[i];
	// }

	fileIn.close();
}

void shutdown() {
	fstream seg("DRIVE/SEGMENT"+std::to_string(segNum)+".txt", fstream::binary );

  seg.write(openBlockInSegment, 1024 * 1024);
  seg.write(reinterpret_cast<const char*>(&summary), 8192);

  seg.close();

  openBlock = 0;

	std::fstream check("DRIVE/CHECKPOINT_REGION.txt", fstream::binary);

  char checkBuf[160];
  memcpy(checkBuf, checkpoint, 160);
  check.write(checkBuf, 160);

  check.close();
}
int main(){
	hardDrive();

	import("other.txt", "DRIVE/SEGMENT0.txt");
	shutdown();

	char buffer[10];
	ifstream inFile("DRIVE/SEGMENT0.txt", ios::binary);
	inFile.read(buffer, 1);

	for(int i = 0; i < 10; i++){
		cout << buffer[i] << endl;
	}

}
