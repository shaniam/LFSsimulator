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
#include "inode.h"
#define KILO 1024
#include <dirent.h>
#include <errno.h>
using namespace std;

vector<char> memorySegment(1048576);
int openBlock = 0;
int segNum = 0;
vector<int> checkpoint(40);
vector<int> imap(40 * KILO);
vector<char> segments(64, 0);
int num = 0;
iNode inode;

void check(){
	if (openBlock == KILO){
		ofstream seg("DRIVE/SEGMENT" + to_string(segNum) + ".txt", ios::binary);
		bool flag = false;

		seg.write(memorySegment.data(), KILO * KILO);

		for (int i = 0; i < 64; i++){
			if (segments.at(i) == 0) {
				segments.at(i) = 1;
				segNum = i + 1;
				flag = true;
			break;
			}
  	}

		seg.close();
		openBlock = 0;

		if(flag == false){
  		cerr << "No more clean blocks!" << endl;

			ofstream checkp("DRIVE/CHECKPOINT_REGION.txt", ios::binary);

  		vector<char> temp(160);
  		memcpy(temp.data(), checkpoint.data(), 160);
			//temp = checkpoint;
  		checkp.write(temp.data(), 160);
  		checkp.write(segments.data(), 64);

			seg.close();
  		checkp.close();
  		exit(-1);
		}
		//cout << "run" << endl;
	}
}

void test(){
	int num = 0;

	for(int x = 0; x < 40; x++){
		ofstream test("test" + to_string(num) + ".txt");
		for(int i = 0; i < 100000 / 8; i++){
			test << rand();
		}
		test.close();
		num++;
	}
}

void hardDrive(){
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

void import(string file, string lfsFile){
	//cout << "seg:" << segNum << endl;
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

	//Find an open spot in the file, fileNameMap was initialized to all 0's so anything that's not a 0 is used
  for (int i = 0; i < 10000; i++){
		// cout << "open: " << openBlock << endl;
		// cout << "I: " << i << endl;
    fileNameMap.seekg(i * 128);
    fileNameMap.read(temp.data(), 1);
		//cout << "segNum: " << segNum << endl;
		//string str(temp.begin(), temp.end());
		//cout << "Str: " << str << endl;

		if (temp.at(0) == '0'){
      iNodeNum = i;
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


	vector<char> inputFileBuffer(KILO);
	//fileIn.read(inputFileBuffer.data(), KILO);
	//memcpy(&memorySegment.at(openBlock * KILO), inputFileBuffer.data(), fileSize);

	for(int i = 0; i <= fileSize / KILO; i++){
		//cout << "open: " << openBlock << endl;
		//fileIn.seekg(i * KILO);
		fileIn.read(inputFileBuffer.data(), KILO);

		//cout << inputFileBuffer.size() << endl;
		// for(int x = 0; x < inputFileBuffer.size(); i++){
		// 	cout << inputFileBuffer[x];
		// }
		memcpy(&memorySegment.at(openBlock * KILO), inputFileBuffer.data(), KILO);
		openBlock++;
		check();
	}
	fileIn.close();
	//memcpy(&memorySegment.at(openBlock * KILO), inputFileBuffer.data(), fileSize);

	// for(int i = 0; i < inputFileBuffer.size(); i++){
	// 	cout << inputFileBuffer[i];
	// }

	// for(int i = 0; i < memorySegment.size(); i++){
	// 	cout << memorySegment[i];
	// }

	//cout << "here" << endl;

	inode.fileName = lfsFile;
	inode.size = fileSize;
	//cout << fileSize / KILO << endl;
	for (int i = 0; i <= (fileSize / KILO); i++){
	  inode.dataBlock[i] = openBlock + segNum * KILO;
	  openBlock++;
	  check();
  }
	//cout << "finished" << endl;
	// cout << inode.fileName << " " << inode.size << " " << inode.dataBlock.data() << endl;
	// cout << iNodeNum << endl;

	// for(int i = 0; i < inode.dataBlock.size(); i++){
	// 	cout << inode.dataBlock[i];
	// }

	memcpy(&memorySegment.at(openBlock * KILO), &inode, sizeof(inode));
	openBlock++;
	check();
	//cout << "open: " << openBlock << endl;

	int frag = iNodeNum / (KILO / 4);
	imap.at(iNodeNum) = (openBlock - 1) + segNum * KILO;
	memcpy(&memorySegment.at(openBlock * KILO), &imap.at(frag * (KILO / 4)), KILO);
	//openBlock++;


	segments.at(segNum) = 1;

	checkpoint.at(frag) = openBlock + segNum * KILO;
	openBlock++;
	check();
	//cout << "open" << openBlock << endl;

	fileNameMap.close();
}

void list(){
	ifstream fileNameMap("DRIVE/FILENAMEMAP.txt");
	vector<char> temp1(1);
	char fileName[128];
	vector<char> temp(sizeof(iNode));

  for (int i = 0; i < 10000; i++){
    //cout << "I: " << i << endl;
    fileNameMap.seekg(i * 128);
    fileNameMap.read(temp1.data(), 1);
    //cout << "Temp" << temp[0] << endl;
    //cout << "I:" << i << endl;
		//cout << "valid: " << valid[0] << endl;
    if (temp1.at(0) != '0'){
      fileNameMap.seekg(i * 128);
      fileNameMap.read(fileName, 128);
			string str(fileName);

			int block = imap[i];
			int segment = block / KILO;
			int local = (block % KILO) * KILO;
			// cout << "Block: " << block << endl;
			// cout << "segment: " << block << endl;
			// cout << "Block: " << block << endl;

		  if(segNum == segment){
		    memcpy(&inode, &memorySegment.at(local), sizeof(iNode));
		  }
			else{
		    ifstream disk("DRIVE/SEGMENT" + to_string(segment) + ".txt", ios::binary);

		    disk.seekg(local);
		    disk.read(temp.data(), sizeof(iNode));
		    memcpy(&inode, temp.data(), sizeof(iNode));

		    disk.close();
		    //cout << "finished" << endl;
			}
			cout << "File Name: " << str << " File Size: " << inode.size << endl;
		 }

		}
  //

  //cout << "SegNum: " << segNum << " segment: " << segment << endl;
			//cout << "open: " << memorySegment.at(local) << endl;
  fileNameMap.close();
}

void shutdown() {
	ofstream seg("DRIVE/SEGMENT" + to_string(segNum)+".txt", ios::binary);
	ofstream checkp("DRIVE/CHECKPOINT_REGION.txt", ios::binary);
	bool flag = false;

	seg.write(memorySegment.data(), 1024 * 1024);

	for (int i = 0; i < 64; i++){
    if (segments.at(i) == 0) {
			segments.at(i) = 1;
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
	exit(0);
}

void removeFunction(string lfsFileName) {
	int iNodeNum = -1;
	vector<char> temp(1);
	char vectFile[128];
	fstream fileNameMap("DRIVE/FILENAMEMAP.txt", ios::in | ios::out);
	int frag;

 	for (int i = 0; i < 10000; i++){
	 	fileNameMap.seekg(i * 128);
	 	fileNameMap.read(temp.data(), 1);

		//cout << "temp: " << temp.at(0) << endl;
	 	if (temp.at(0) != '0') {
			fileNameMap.seekg(i * 128);
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
		fstream seg("DRIVE/SEGMENT" + to_string(segNum) + ".txt", ios::binary | ios::in | ios::out);
		ofstream checkp("DRIVE/CHECKPOINT_REGION.txt", ios::binary);
		bool flag = false;
		int frag;

		seg.write(memorySegment.data(), KILO * KILO);

		for (int i = 0; i < 64; ++i){
	    if (segments.at(i) == 0) {
				segments.at(i) = 1;
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
		 seg.close();
        checkp.close();
		}
	}

	//cout << "inodenum: " << iNodeNum << endl;
	//cout << "test: " << imap.at(iNodeNum) << endl;

  imap.at(iNodeNum) = 0;

 	frag = iNodeNum / (KILO / 4);

  memcpy(&memorySegment.at(openBlock * KILO), &imap.at(frag * (KILO / 4)), KILO);

  checkpoint[frag] = openBlock + segNum * KILO;

  openBlock++;

	;

}

void restart(){
	fstream checkp("DRIVE/CHECKPOINT_REGION.txt", ios::binary | ios::out | ios::in);
	vector<char> temp(160);
	vector<char> temp2(KILO);
	int segment, block;
	bool flag = false;
	int num = 0;

	// seg.write(memorySegment.data(), 1024 * 1024);
	//
	// for (int i = 0; i < 64; i++){
  //   if (segments.at(i) == 0) {
  //     segNum = i + 1;
	// 		flag = true;
  //     break;
  //   }
  // }
	//
	// if(flag == false){
	// 	cerr << "No more clean blocks!" << endl;
	//
	// 	vector<char> tem(160);
	// 	memcpy(tem.data(), checkpoint.data(), 160);
	// 	checkp.write(tem.data(), 160);
	// 	checkp.write(segments.data(), 64);
	//
	// 	checkp.close();
	//   seg.close();
	// 	exit(-1);
	// }

	char buffer[160];
	checkp.read(buffer, 160);

	memcpy(checkpoint.data(), buffer, 160);

	checkp.read(segments.data(), 64);

	checkp.close();

  for(int i = 0; i < 40; i++){
    if(checkpoint.at(i) >= 0 && checkpoint.at(i) >= num){
      num = checkpoint.at(i);
      flag = true;
    }
  }

	if(flag){
		openBlock = (num % KILO) + 1;
	}
	else{
		openBlock = 0;
	}

	segNum = num / KILO;

	fstream seg("DRIVE/SEGMENT" + to_string(segNum) + ".txt", ios::binary | ios::in);

  seg.read(memorySegment.data(), KILO * KILO);

	seg.close();

	for (int i = 0; i < 40; i++){
		if (checkpoint.at(i) != 0){
			segment = checkpoint.at(i) / KILO;
			block = (checkpoint.at(i) % KILO) * KILO;
			fstream segg("DRIVE/SEGMENT" + to_string(segment) + ".txt", ios::binary | ios::in | ios::out);

			segg.seekg(block);
			segg.read(temp2.data(), KILO);
			memcpy(&imap.at(i * (KILO / 4)), temp2.data(), KILO);


			seg.close();
		}
  }

	checkp.close();
}

void display(string lfs_filename, string total, string starter){
	fstream fileNameMap("DRIVE/FILENAMEMAP.txt", ios::in | ios::out);
	vector<char> temp(128);
	char str[128];
	int iNodeNum = -1;
	int howmany = stoi(total);
	int start = stoi(starter);
	int setFlag = false;

	//Find an open spot in the file, fileNameMap was initialized to all 0's so anything that's not a 0 is used
  for (int i = 0; i < 10000; i++){
	    fileNameMap.seekg(i * 128);
	    char valid[1];
	    fileNameMap.read(valid, 1);

	    if (valid[0] != '0'){
	      char filename_buffer[127];
	      fileNameMap.read(filename_buffer, 127);

	      std::string filename(filename_buffer);
	      if (filename == lfs_filename){
	        fileNameMap.close();
	        iNodeNum = i;
					break;
	      }
	    }
	  }

		if(iNodeNum == -1){
			cerr << "File not found!" << endl;
			exit(-1);
		}

		cout << "inode: " << iNodeNum << endl;

	fileNameMap.close();

	int block = imap[iNodeNum];
  int segment = block / KILO;
  int local = block;

	cout << "block: " << block << endl;
	cout << "segment: " << segment << endl;
	cout << "local: " << local << endl;

  if (segment != segNum){
    ifstream seg("DRIVE/SEGMENT" + to_string(segment) + ".txt", ios::binary);

    seg.seekg(local);
    char buffer[KILO];
    seg.read(buffer, KILO);

    memcpy(&inode, buffer, sizeof(inode));

    seg.close();
  }else{
    memcpy(&inode, &memorySegment[local], sizeof(inode));
	}

	//printBlock(meta.block_locations[i], start_byte, end_byte, (i == start_byte/BLOCK_SIZE), (i == end_byte/BLOCK_SIZE));

	local += 1023;

	for (int i = start; i <= (howmany + start); i++){
		// int segment = (inode.dataBlock[i] / KILO);
		// int local = (inode.dataBlock[i] % KILO) * KILO;

		// cout << "Segment: " << segment << endl;
		// cout << "Local: " << local << endl;



  	// if (i == start / KILO){
		// 	local += start;
		// }

	  unsigned int buffer_size = 1;

	  // if ((i == start / KILO) && (i == (howmany + start) / KILO)){
		// 	buffer_size = (howmany + start) - start;
		// }
	  // else if (i == (howmany + start) / KILO){
		// 	buffer_size = (howmany + start) % KILO;
		// }
	  // else if (i == start / KILO){
		// 	buffer_size = KILO - (start % KILO);
		// }
	  // else{
		// 	buffer_size = KILO;
		// }

	  char buffer[1];

	  if (segment != segNum){
	    ifstream segg("DRIVE/SEGMENT" + to_string(segment) + ".txt", ios::binary);

	    segg.seekg(local);
	    segg.read(buffer, 1);

	    segg.close();
	  }else{
	    memcpy(buffer, &memorySegment[local], 1);
	  }

	  cout << buffer[0];
		local++;
		// if(local == (howmany + start)){
		// 	cout << endl;
		// }
	}
}

int main(){

	 srand(time(NULL));

	 //test();

	DIR* dir = opendir("DRIVE");
	if (dir)
	{
    /* Directory exists. */
    	closedir(dir);
	//drive();
	}
	else if (ENOENT == errno)
	{
	cerr << "drive does not exist" << endl;
	hardDrive();
	test();
    /* Directory does not exist. */
	}
	else
	{
    /* opendir() failed for some other reason. */
	}

//	 hardDrive();
	 restart();
	 string mystr;
	 cerr << "please enter your command in the following formats!  'display <lfs_filename> <howmany> <start>', 'list' , 'remove <lfs_filename>' , 'import <filename> <lfs_filename>', 'shutdown', , to exit enter 'exit' " << endl;

	 while (getline(cin, mystr)){
	 //cerr << "please enter your command in the following formats!  'list' , 'remove <lfs_filename>' , 'import <filename> <lfs_filename>', 'shutdown', 'restart' to exit enter 'exit' " << endl;
	 //cerr << mystr << endl;
	 if (mystr.compare("exit")==0){
	 exit(-1);
	 }
	 else if(mystr.compare("list")==0){

	 list();
	 }
	 else if(mystr.compare("shutdown")==0){
	 shutdown();
	 }
	 /*else if (mystr.compare("restart")==0){
	 restart();
	 }*/
	 else{

	 string word;
	 string filen1;
	 string filen2;
	 vector<string> vecs={};
	 for (auto x: mystr){
	 if (x == ' '){
	 vecs.push_back(word);
	 word="";}
	 else{
	 word=word+x;
	 }
	 }
	/*for (auto x: vecs){
	cerr << x << endl;
	}*/
	 vecs.push_back(word);
	 //cerr << vecs[0] << endl;
	 //cerr << vecs.size() << endl;
	 if (vecs[0].compare("import")==0 && vecs.size()==3){
		 cerr << "import" << endl;
		 import(vecs[1], vecs[2]);
	 }
	 else if(vecs[0].compare("remove")==0 && vecs.size()==2){
		 cerr << "remove" << endl;
		 removeFunction(vecs[1]);
	 }
	 else if(vecs[0].compare("display") == 0 && vecs.size() == 4){
		 cerr << "display" << endl;
		 display(vecs[1], vecs[2], vecs[3]);
		 cerr << endl;
	 }
	 else{
	 cerr << "invalid command" << endl;
	 continue;
	 }
	 }
	 cerr << "please enter your command in the following formats!  'display <lfs_filename> <howmany> <start>' 'list' , 'remove <lfs_filename>' , 'import <filename> <lfs_filename>', 'shutdown',  to exit enter 'exit' " << endl;

	 }

// //test();
//
	//~ import("test0.txt", "num0test.txt");
	//~ list();
//
	//srand(time(NULL));

	//test();

	//~ hardDrive();

	//~ import("other.txt", "hello.txt");
	//~ import("check.txt", "bye.txt");
	//~ for(int i = 0; i < 40; i++){
		//~ import("test" + to_string(i) + ".txt", "num" + to_string(i) + "test.txt");
	//~ }
	//~ cout << "list with everything" << endl;
	//~ list();
	//~ cout << "removing bye.txt" << endl;
	//~ removeFunction("bye.txt");
	//~ srand(time(NULL));

	 //~ cout << "removing 5 random files" << endl;
	 //~ for(int i = 0; i < 5; i++){

	 	//~ int y = rand() % 40;
	 	//~ cout << "removing num" << y << "test.txt" << endl;
		//~ removeFunction("num" + to_string(y) + "test.txt");
	 //~ }

	//~ cout << "list after 5 removal" << endl;
	//~ list();

	//~ cout << "shutting down" << endl;
	//~ shutdown();
	//~ list();
	 //~ cout << "before restart" << endl;
	//~ //int * before = imap.data();
	//~ // for (int i = 0; i < imap.size(); ++i){
  //~ //       beforeFile << *before++ << " ";
	//~ // 		}
	//~ cout << "restarting" << endl;
	//~ restart();
	//~ //cout << "after restart" << endl;
	//~ // int * after = imap.data();
	//~ // for (int i = 0; i < imap.size(); ++i){
  //~ //       afterFile << *after++ << " ";
	//~ // 		}
	//~ list();

	// for(int i = 20; i < 40; i++){
	// 	import("test" + to_string(i) + ".txt", "num" + to_string(i) + "test.txt");
	// }
	//
	// list();

	// char buffer[KILO * KILO];
	// fstream inFile("DRIVE/SEGMENT0.txt", ios::binary | ios::in);
	// inFile.read(buffer, KILO * KILO);
	//
	// for(int i = 0; i < KILO * KILO; i++){
	// 	cout << buffer[i];
	// }

}
