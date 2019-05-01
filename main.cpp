#include <stdlib.h>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <queue>
#include <fstream>
#include <algorithm>
#include <string>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <utility>
#define KILO 1024

using namespace std;

int inodes=10*KILO;
int mapindex=0;
vector<int> imap(inodes,-1);
vector<char>inMem(KILO*KILO, '0');
int file=0;
int inmem_index=0;
int segindex=KILO;
vector<int> ssbnodes={};

/*void hardDrive(){
	if (mkdir("DRIVE", 0777));
	else{
	}

	string name="DRIVE/SEGMENT";
	for (int i=0; i<64; i++){
	string iter=to_string(i);
	string file=name+iter+".txt";
	ofstream outfile(file);
	if(!outfile.is_open()){
		cerr << "oh no!" << endl;
		return;
	}
	int y=0;
	int* x=&y;
	char* z=(char*) x;
	for (int x=0; x<1048576; x++){
		//outfile << '0';
		outfile.write(z, 1);
	}
	outfile.close();
	}
}*/
void hardDrive(){
	if (mkdir("DRIVE", 0777));
	else{
		}

	string name="DRIVE/SEGMENT";

	for (int i=0; i<64; i++){
		string iter=to_string(i);
		string file=name+iter+".txt";
		ofstream outfile(file, ios::binary);
		if(!outfile.is_open()){
			cerr << "oh no!" << endl;
			return;
		}

		int y = 0;

		for (int x = 0; x < 1048576 / 4; x++){
			outfile.write(reinterpret_cast<const char*>(&y), sizeof(y));
		}

		outfile.close();
	}
}

class inode{
	public:
		string name;
		int size;
		vector<int> blocks={};
		inode(string names){
			name=names;
			ifstream in_file(name, ios::binary);
			in_file.seekg(0, ios::end);
			size=in_file.tellg();
			in_file.close();
			for (int i=0; i<128; i++){
				blocks.push_back(-1);
			}
		}
};


void import(string file1, string file2){
	ifstream src;
	ofstream dst;

	src.open(file1, ios::in | ios::binary);
  	dst.open(file2, ios::out | ios::binary);
  	dst << src.rdbuf();

	src.close();
	dst.close();
        inode* node= new inode(file2);
	imap[mapindex]=file+segindex;
	int i=0;
	string node_info;
	#if 0
	/*=node->name+to_string(node->size);
	/*for (int x: node->blocks){
		node_info+=to_string(x);
	}
	int inode_loc=segindex;
	int curr_file=file;
	for(char x: node_info){
		inMem[segindex+i]=x;
		i++;
	}
	/*for (auto x: inode->name);
		inMem[segindex+i]=x;*/
	mapindex++;
	segindex+=1024;
	#endif
	//char* x=inMem;
	ifstream in;
	in.open(file2);
	//int i=0;
	char x;
	vector<int> blocks_1(128,-1);
	while(in.read(&x ,1)){
		inMem[segindex+i]=x;
		i++;
		if(segindex==KILO*KILO){
		string filen="DRIVE/SEGMENT"+to_string(file)+".txt";
		dst.open(filen);
		//dump;
		for (auto x: inMem){
			dst << x;
		file++;
		segindex=1024;
		inMem.clear();
		}
		dst.close();
		cerr << "houstin, we have a problem" << endl;
		break;
		}
		if(segindex%1024==0){
			blocks_1.push_back(file*KILO+segindex);
			//update inode;
		}
	}
	node->blocks=blocks_1;
	fstream dst1;
	int result=((file*KILO+segindex+KILO/2)/KILO)*KILO;
	imap[mapindex]=result;
	string filen="DRIVE/SEGMENT"+to_string(file)+".txt";
	dst1.open(filen);
	dst1.ignore(file*KILO+segindex);
	node_info=node->name+to_string(node->size);
        for (int x: node->blocks){
                node_info+=to_string(x);
        }
        //int inode_loc=segindex;
        //int curr_file=file;
        i=0;
	for(char x: node_info){
                //dst1<<x;

		inMem[segindex+i]=x;
                i++;
	         //i++;
        }
	dst1.close();

	//if(segindex==1024*1024);

	#if 0
	//pair<int, int> loc=find();
 	string name="DRIVE/SEGMENT";
	name=name+to_string(loc.first)+".txt";
	fstream out(name);
	out.ignore(loc.second);
	out << (node->name);
	//cerr << (node->size);
	out.close();

        /*infile.open(file);
        if (!infile){
                exit(1);
        }*/
        //find();
        /* insert into segments*/

        //infile.close();
	ifstream in(node->name);
	//out.open(name);
	while(in.is_open()){
		loc=find();
		out.open(name);
		//ifstream in(node->name);
		if (!out.is_open()){
			cerr << "out not open" << endl;
		}
		if (!in.is_open()){
			cerr << "in not open" << endl;
		}
		out.ignore(loc.second);
		cerr << loc.second << endl;
		char x;
		int i=0;
		while(in.read(&x,1)){
			out << x;
			i++;
			if (i==KILO){
			break;
			}
		}
		out.close();
	}

	/*for (auto x: inMem){
	cerr << x << endl;
	}*/
	in.close();
	#endif
}
int main(){
	for (int i=0; i<inodes; i++){
		imap[i]=-1;
	}

	hardDrive();
	//pair<int, int> test=find();
	import("other.txt", "hi.txt");

}
