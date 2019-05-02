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
#include "inode.h"
#define KILO 1024

using namespace std;

vector<char> openBlockInSegment(1048576);
int openBlock = 0, segNum = 0;
vector<vector<int>> summary(KILO, vector<int>(2));
vector<int> checkpoint(40);
vector<int> imap(40 * KILO);
vector<char> segments(64, 0);

void hardDrive(){
	mkdir("DRIVE", 0777);

	int y = 0;
	string name = "DRIVE/SEGMENT";
	string checkpointstr = "DRIVE/CHECKPOINT_REGION";
	string fileNameMap = "DRIVE/FILENAMEMAP";
	ofstream outfile2(checkpointstr+".txt", ios::binary);
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
	ifstream fileIn(file, ios::binary);
  if (!fileIn.is_open()){
    cout << "File not found!" << endl;
    exit(-1);
  }

	//Determine size of file
  fileIn.seekg(0, ios::end);
  int fileSize = fileIn.tellg();
  fileIn.seekg(0, ios::beg);

	fstream fileNameMap("DRIVE/FILENAMEMAP.txt", ios::in | ios::out);
	vector<char> temp(1);
	int iNodeNum = -1;
	int frag = iNodeNum / (KILO / 4);

	//Find an open spot in the file, fileNameMap was initialized to all 0's so anything that's not a 0 is used
  for (int i = 0; i < 10000; i++){
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
	inode.fileName = lfsFile;
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
		ofstream seg("DRIVE/SEGMENT" + to_string(segNum) + ".txt", ios::binary);
		bool flag = false;

		seg.write(openBlockInSegment.data(), KILO * KILO);
		seg.write(reinterpret_cast<const char *>(&summary), 8192);

		for (int i = 0; i < 64; i++){
    	if (segments.at(i) == 0) {
      	segNum = i +1;
				flag = true;
      	break;
    	}
  	}

		if(flag == false){
  		cerr << "No more clean blocks!" << endl;

			ofstream checkp("DRIVE/CHECKPOINT_REGION.txt", ios::binary);

  		vector<char> temp(160);
  		memcpy(temp.data(), checkpoint.data(), 160);
			//temp = checkpoint;
  		checkp.write(temp.data(), 160);
  		checkp.write(segments.data(), 64);

  		checkp.close();
  		exit(-1);
		}

		seg.close();
		//cout << "run" << endl;
	}

  imap.at(iNodeNum) = (openBlock - 1) + (segNum * KILO);

  memcpy(&openBlockInSegment.at(openBlock * KILO), &imap.at(frag * (KILO / 4)), KILO);

  summary.at(openBlock).at(0) = -1;
  summary.at(openBlock).at(1) = frag;

	segments.at(segNum) = 1;

  checkpoint.at(frag) = openBlock + (segNum * KILO);

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
	ifstream fileNameMap("DRIVE/FILENAMEMAP.txt");
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

			int block = imap[i];
			int segment = block / KILO;
			int local = (block % KILO) * KILO;
			// cout << "Block: " << block << endl;
			// cout << "segment: " << block << endl;
			// cout << "Block: " << block << endl;

		  iNode inode1;

		  if(segNum == segment){
				memcpy(&inode1, &openBlockInSegment[local], sizeof(iNode));
		  }
			else{
		    ifstream disk("DRIVE/SEGMENT" + to_string(segment) + ".txt", ios::binary);

		    disk.seekg(local);
		    char buffer[sizeof(inode1)];
		    disk.read(buffer, sizeof(inode1));
		    memcpy(&inode1, buffer, sizeof(inode1));

		    disk.close();
			}
			cout << "File Name: " << str << " File Size: " << inode1.size << endl;
		  }
		}

			//cout << "SegNum: " << segNum << " segment: " << segment << endl;
			//cout << "open: " << openBlockInSegment.at(local) << endl;

  fileNameMap.close();
}

void shutdown() {
	ofstream seg("DRIVE/SEGMENT"+to_string(segNum)+".txt", ios::binary);
	ofstream checkp("DRIVE/CHECKPOINT_REGION.txt", ios::binary);
	bool flag = false;

  seg.write(openBlockInSegment.data(), 1024 * 1024);
  seg.write(reinterpret_cast<const char*>(&summary), 8192);

	for (int i = 0; i < 64; i++){
    if (segments.at(i) == 0) {
      segNum = i + 1;
			flag = true;
      break;
    }
  }

	if(flag == false){
		cerr << "No more clean blocks!" << endl;

		vector<char> temp(160);
		memcpy(temp.data(), checkpoint.data(), 160);
		checkp.write(temp.data(), 160);
		checkp.write(segments.data(), 64);

		checkp.close();
	  seg.close();
		exit(-1);
	}

  openBlock = 0;

  vector<char> temp(160);
  memcpy(temp.data(), checkpoint.data(), 160);
  checkp.write(temp.data(), 160);
	checkp.write(segments.data(), 64);

	checkp.close();
  seg.close();
	//exit(0);
}

void removeFunction(string lfsFileName) {
	int iNodeNum = -1;
	char temp[1];
	fstream fileNameMap("DRIVE/FILENAMEMAP.txt", ios::in | ios::out);

 	for (int i = 0; i < 10000; i++){
	 	fileNameMap.seekg(i * 128);
	 	fileNameMap.read(temp, 1);

		//cout << "temp: " << temp.at(0) << endl;
	 	if (temp[0] != '0') {
			fileNameMap.seekg(i * 128);
		 	char vectFile[128];
		 	fileNameMap.read(vectFile, 128);
		 	string strFileName(vectFile);

			//cout << "str: " << strFileName << " lfs: " << lfsFileName << endl;

		 	if (strcmp(lfsFileName.c_str(), strFileName.c_str()) == 0){
			 	iNodeNum = i;
				break;
		 	}
	 	}
 	}

 	if(iNodeNum == -1){
	 	cerr << "File Name Not Found!" << endl;
	 	exit(-1);
 	}

	char zero[128] = {'0'};

  fileNameMap.seekp(iNodeNum * 128);
  fileNameMap.write(zero, 128);

	fileNameMap.close();

	if(openBlock == KILO){
		fstream seg("DRIVE/SEGMENT"+to_string(segNum)+".txt", ios::binary | ios::in | ios::out);
		ofstream checkp("DRIVE/CHECKPOINT_REGION.txt", ios::binary);
		bool flag = false;
		int frag;

		seg.write(openBlockInSegment.data(), KILO * KILO);
		seg.write(reinterpret_cast<const char*>(&summary), 8192);

		for (int i = 0; i < 64; ++i){
    if (segments.at(i) == 0) {
      segNum = i + 1;
			flag = true;
      break;
    }

		if(flag == false){
  		cerr << "No more clean blocks!" << endl;

  		vector<char> temp(160);
  		memcpy(temp.data(), checkpoint.data(), 160);
  		checkp.write(temp.data(), 160);
  		checkp.write(segments.data(), 64);

  		exit(-1);
		}

		openBlock = 0;
	}

	//cout << "inodenum: " << iNodeNum << endl;
	//cout << "test: " << imap.at(iNodeNum) << endl;

  imap.at(iNodeNum) = -1;

 	frag = iNodeNum / (KILO / 4);

  memcpy(&openBlockInSegment.at(openBlock * KILO), &imap.at(frag * (KILO / 4)), KILO);

  summary.at(openBlock).at(0) = -1;
  summary.at(openBlock).at(1) = frag;

  checkpoint[frag] = openBlock + segNum * KILO;

  openBlock++;

	seg.close();
	checkp.close();
}
}

void restart(){
	fstream checkp("DRIVE/CHECKPOINT_REGION.txt", ios::binary | ios::out | ios::in);
	fstream seg("DRIVE/SEGMENT" + to_string(segNum) + ".txt", ios::binary | ios::in | ios::out);
	bool flag = false;
  vector<char> temp(160);
	vector<char> temp1(8192);
	vector<char> temp2(KILO);
	int segment, block;
	int num = 0;

  checkp.read(temp.data(), 160);
  memcpy(checkpoint.data(), temp.data(), 160);

  checkp.read(segments.data(), 64);

  for(int i = 0; i < 40; i++){
    if(checkpoint.at(i) != -1 && checkpoint.at(i) >= num){
      num = checkpoint.at(i);
      flag = true;
    }
  }

	segNum = 1 + (num / KILO);

	if(flag == true){
		openBlock = (num % KILO) + 1;
	}
	else{
		openBlock = 0;
	}

  seg.read(openBlockInSegment.data(), KILO * KILO);
  seg.read(temp1.data(), 8192);
  memcpy(summary.data(), temp1.data(), 8192);

	for (int i = 0; i < 40; i++){
    if (checkpoint.at(i) != -1){
			segment = (checkpoint.at(i) / KILO) + 1;
			block = (checkpoint.at(i) % KILO) * KILO;

			seg.seekg(block);
			seg.read(temp2.data(), KILO);
			memcpy(&imap.at(i * (KILO / 4)), temp2.data(), KILO);
		}
  }

  seg.close();
	checkp.close();
}

int main(){
	ofstream beforeFile("before.txt", ios::binary);
	ofstream afterFile("after.txt", ios::binary);

	hardDrive();

	import("other.txt", "hello.txt");
	import("check.txt", "bye.txt");
	cout << "1" << endl;
	list();
	cout << "removing bye.txt" << endl;
	removeFunction("bye.txt");

	//cout << "2" << endl;
	//list();
	//shutdown();

	//cout << "shutting down" << endl;
	//shutdown();
	//list();
	cout << "before restart" << endl;
	int * before = imap.data();
	for (int i = 0; i < imap.size(); ++i){
        beforeFile << *before++ << " ";
			}
	cout << "restarting" << endl;
	restart();
	cout << "after restart" << endl;
	int * after = imap.data();
	for (int i = 0; i < imap.size(); ++i){
        afterFile << *after++ << " ";
			}
	//list();

	// char buffer[4096];
	// fstream inFile("DRIVE/SEGMENT0.txt", ios::binary | ios::in);
	// inFile.read(buffer, 4096);
	//
	// for(int i = 0; i < 4096; i++){
	// 	cout << buffer[i];
	// }

}
