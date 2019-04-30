
#include <stdlib.h>
#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <queue>
#include <fstream>
#include <algorithm>
#include <string>
#include <bits/stdc++.h> 
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


void hardDrive(){
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
	for (int x=0; x<1048576; x++){
		outfile << '0';
	}
	outfile.close();
	}
}
class inode{
	public:
		string name;
		int size;
		vector<int> blocks={};
		//blocks.reserve(128);
		inode(string names){
			name=names;
			//fseek(fp, 0L,SEEK_END);
			std::ifstream in_file(name, ios::binary);
			in_file.seekg(0, ios::end);
			size=in_file.tellg();
			in_file.close();
			//blocks.reserve(128);
			for (int i=0; i<128; i++){
				blocks.push_back(-1);
			}
		}
};

pair<int, int> find(){
	ifstream in;
	string name="DRIVE/SEGMENT";
	char data;
	bool good=true;
	int block;
	for (int i=63; i>=0; i--){
		char data='P';
		char* other=&data;
		string iter=to_string(i);
        	string file=name+iter+".txt";
		int block=1047552;
		while (block>=0){
			in.open(file);
			in.ignore(block);
			//block=0;
			good=true;
			for (int x=0; x<1024; x++){
                		in.read(other, 1);
				if (data!='0'){
					good=false;
					break;
				}
				//block=x;
				//
        		}
		if (good==true){return make_pair(i,block);}
		in.close();
		block-=1024;
		}
		if (good==true){
			return make_pair(i, block);
		}
		else{
		continue;
		}

	}

}
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
	string node_info=node->name+to_string(node->size);
	for (int x: node->blocks){
		node_info+=to_string(x);
	}
	for(char x: node_info){
		inMem[segindex+i]=x;
		i++;
	}
	/*for (auto x: inode->name);
		inMem[segindex+i]=x;*/
	mapindex++;
	segindex+=1024;
	//char* x=inMem;        
	ifstream in;
	in.open(file2);
	i=0;
	char x;
	while(in.read(&x ,1)){
		inMem[segindex+i]=x;
		i++;
		if(segindex==KILO*KILO){
		//dump;
		/*for (auto x: inMem){
			
		}*/
		cerr << "houstin, we have a problem" << endl;
		break;
		}
		if(segindex%1024==0){
			//update inode;
		}
	}
	ofstream check;
	check.open("check.txt");
	for (auto x: inMem){
        check << x ;
        }
	in.close();
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