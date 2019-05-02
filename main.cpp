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

vector<char> openBlockInSegment(1048576);
int openBlock = 0;
int segNum = 0;
vector<vector<int>> summary(KILO, vector<int>(2));
vector<int> checkpoint(40);
vector<int> imap(40 * KILO);

int getFileSize(int i);

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

	for(int i = 0; i < KILO; i++){
		summary.at(i).reserve(2);
	}
}

void import(string file, string lfsFile){
	fstream fileIn(file, ios::binary | ios::in);
  if (!fileIn.is_open()){
    cout << "File not found!" << endl;
    exit(-1);
  }

	//Determine size of file
  fileIn.seekg(0, ios::end);
  int fileSize = fileIn.tellg();
  fileIn.seekg(0, ios::beg);

	if ((fileSize / KILO) > KILO - openBlock){
		fstream seg("DRIVE/SEGMENT"+to_string(segNum)+".txt", ios::binary | ios::out);

		seg.write(openBlockInSegment.data(), KILO * KILO);
		seg.write(reinterpret_cast<const char*>(&summary), 8192);

		seg.close();

		openBlock = 0;
}

	fstream fileNameMap("DRIVE/FILENAMEMAP.txt", ios::in | ios::out);
	vector<char> temp(1);
	int iNodeNum = -1;
	int frag = iNodeNum / (KILO / 4);

	//Find an open spot in the file, fileNameMap was initialized to all 0's so anything that's not a 0 is used
  for (int i = 0; i < 10000; i ++){
    fileNameMap.seekg(i * 128);
		//fill(begin(temp), end(temp), '0');
    fileNameMap.read(temp.data(), 1);

		//string str(temp.begin(), temp.end());
		//cout << "Str: " << str << endl;

		//Found an empty spot in the fileNameMap
		if (temp.at(0) == '0'){
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
	int n = lfsFile.length() + 1;
  char fileCArray[n];
  strcpy(fileCArray, lfsFile.c_str());

	//Find proper spot in file (128byte blocks), write the name, and then fill remainder of block with garbage
	fileNameMap.seekp(iNodeNum * 128);
	fileNameMap.write(fileCArray, n);

	vector<char> inputFileBuffer(fileSize);
	fileIn.read(inputFileBuffer.data(), fileSize);
	memcpy(&openBlockInSegment.at(openBlock * KILO), inputFileBuffer.data(), fileSize);

	// for(int i = 0; i < inputFileBuffer.size(); i++){
	// 	cout << inputFileBuffer[i];
	// }

	// for(int i = 0; i < openBlockInSegment.size(); i++){
	// 	cout << openBlockInSegment[i];
	// }

	iNode inode;
	inode.fileName = file;
	inode.size = fileSize;
	for (int i = 0; i <= (fileSize / KILO); i++){
    inode.dataBlock[i] = openBlock + (segNum * KILO);
    summary.at(openBlock).at(0) = iNodeNum;
    summary.at(openBlock).at(1) = i;
    openBlock++;
		//cout << i << endl;
  }


	// cout << inode.fileName << " " << inode.size << " " << inode.dataBlock.data() << endl;
	// cout << iNodeNum << endl;

	// for(int i = 0; i < inode.dataBlock.size(); i++){
	// 	cout << inode.dataBlock[i];
	// }

	summary.at(openBlock).at(0) = iNodeNum;
	summary.at(openBlock).at(1) = -1;

	memcpy(&openBlockInSegment.at(openBlock * KILO), &inode, sizeof(inode));
	openBlock++;

	if (openBlock == KILO){
		fstream seg("DRIVE/SEGMENT"+to_string(segNum)+".txt", ios::binary | ios::out);

		seg.write(openBlockInSegment.data(), KILO * KILO);
		seg.write(reinterpret_cast<const char*>(&summary), 8192);

		seg.close();
		//cout << "run" << endl;
	}

  imap.at(iNodeNum) = (openBlock - 1) + (segNum * KILO);

  memcpy(&openBlockInSegment.at(openBlock * KILO), &imap.at(frag * (KILO / 4)), KILO);

  summary.at(openBlock).at(0) = -1;
  summary.at(openBlock).at(1) = frag;

  checkpoint.at(frag) = openBlock + segNum * KILO;

  openBlock++;

	// fstream in("DRIVE/SEGMENT0.txt", ios::binary);
	//
	// char buffer[fileSize];
	// in.read(buffer, fileSize);
	//
	// for(int i = 0; i < fileSize; i++){
	// 	cout << buffer[i];
	// }

	fileIn.close();
}

void list(){
	fstream fileNameMap("DRIVE/FILENAMEMAP.txt", ios::in);
	vector<char> temp(1);

  for (int i = 0; i < 10000; i++){
    fileNameMap.seekg(i * 128);
    fileNameMap.read(temp.data(), 1);

		//cout << "valid: " << valid[0] << endl;
    if (temp.at(0) != '0'){
			fileNameMap.seekg(i * 128);
			char fileName[128];
      fileNameMap.read(fileName, 128);
			string str(fileName);
			//
		 	printf("File Name: %s File Size: %d\n", fileName, getFileSize(i));
		}
	}
			//cout << "Block: " << block << endl;
			//cout << "segment: " << block << endl;
			//cout << "Block: " << block << endl;

			//cout << "SegNum: " << segNum << " segment: " << segment << endl;
			//cout << "open: " << openBlockInSegment.at(local) << endl;

  fileNameMap.close();
}

void shutdown() {
	fstream seg("DRIVE/SEGMENT"+to_string(segNum)+".txt", ios::binary | ios::out);

  seg.write(openBlockInSegment.data(), 1024 * 1024);
  seg.write(reinterpret_cast<const char*>(&summary), 8192);

  seg.close();

  openBlock = 0;

	fstream check("DRIVE/CHECKPOINT_REGION.txt", ios::binary | ios::out);

  vector<char> checkBuf(160);
  memcpy(checkBuf.data(), checkpoint.data(), 160);
  check.write(checkBuf.data(), 160);

  check.close();
	exit(0);
}

int getFileSize(int i){
	int block = imap[i];
	int segment = block / KILO;
	int local = (block % KILO) * KILO;

  iNode inode1;

  if(segNum == segment){
		memcpy(&inode1, &openBlockInSegment[local], sizeof(iNode));
  }
	else{
    fstream disk("DRIVE/SEGMENT" + std::to_string(segment) + ".txt", std::ios::binary | std::ios::in);

    disk.seekg(local);
    char buffer[sizeof(inode1)];
    disk.read(buffer, sizeof(inode1));
    memcpy(&inode1, buffer, sizeof(inode1));

    disk.close();
  }

  return inode1.size;
}

void removeFunction(string lfsFileName) {
	int iNodeNum = -1;
	vector<char> temp(1);
	fstream fileNameMap("DRIVE/FILENAMEMAP.txt", ios::in | ios::out);

 	for (int i = 0; i < 10000; i++){
	 	fileNameMap.seekg(i * 128);
	 	fileNameMap.read(temp.data(), 1);

		//cout << "temp: " << temp.at(0) << endl;
	 	if (temp.at(0)) {
		 	vector<char> vectFile(128);
		 	fileNameMap.read(vectFile.data(), 128);
		 	string strFileName(vectFile.begin(), vectFile.end());

		 	if (strFileName == lfsFileName){
			 	iNodeNum = i;
				break;
		 	}
	 	}
 	}

 	if(iNodeNum == -1){
	 	cerr << "File Name Not Found!" << endl;
	 	exit(-1);
 	}

	char zero[128] = {0};

  fileNameMap.seekp(iNodeNum * 128);
  fileNameMap.write(zero, 128);

	fileNameMap.close();

	if(openBlock == KILO){
		fstream seg("DRIVE/SEGMENT"+to_string(segNum)+".txt", ios::binary | ios::in | ios::out);

		seg.write(openBlockInSegment.data(), KILO * KILO);
		seg.write(reinterpret_cast<const char*>(&summary), 8192);

		seg.close();

		openBlock = 0;
	}

  imap[iNodeNum] = -1;

 	int frag = iNodeNum / (KILO / 4);

  memcpy(&openBlockInSegment.at(openBlock * KILO), &imap.at(frag * (KILO / 4)), KILO);

  summary.at(openBlock).at(0) = -1;
  summary.at(openBlock).at(1) = frag;

  checkpoint[frag] = openBlock + (segNum - 1) * KILO;

  openBlock++;
}
int main(){
	hardDrive();

	import("other.txt", "hello.txt");
	import("check.txt", "bye.txt");
	remove("DRIVE/SEGMENT0.txt");

	list();
	//shutdown();

	// char buffer[1024];
	// ofstream inFile("DRIVE/SEGMENT0.txt", ios::binary);
	// inFile.read(buffer, 1024);
	//
	// for(int i = 0; i < 1024; i++){
	// 	cout << buffer[i];
	// }

}
