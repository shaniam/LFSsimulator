
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
using namespace std;
void hardDrive(){
	//ofstream  segment;
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
		//continue;
		return;
	}
	for (int x=0; x<1048576; x++){
		outfile << '0';
	}
//	}
	outfile.close();
	}
}
class inode{
	public:
		string name;
		int size;
		vector<int*> blocks={};
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
				blocks.push_back(NULL);
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
		int block=990000;
		while (block>=0){
			in.open(file);
			in.ignore(block);
			//block=0;
			good=true;
			for (int x=0; x<1048576; x++){
                		in.read(other, 1);
				if (data!='0'){
					good=false;
					break;
				}
				//block=x;
				//
        		}
			if (good==true){return make_pair(i,block);}
			block-=10000;
		}
		if (good==true){
			return make_pair(i, block);
		}
		else{
		continue;
		}

	}

}
void import(string file){
        inode* node= new inode(file);

        pair<int, int> loc=find();
 	string name="DRIVE/SEGMENT";
	name=name+to_string(loc.first)+".txt";
	fstream out(name);
	out.ignore(loc.second);
	out << (node->name);
	cerr << (node->size);
	out.close();

        /*infile.open(file);
        if (!infile){
                exit(1);
        }*/
        //find();
        /* insert into segments*/

        //infile.close();
	loc=find();
	ifstream in(node->name);
	out.open(name);
	cerr << name << endl;
	out.ignore(loc.second);
	char x;
	while(in.read(&x,1)){
		out << x;
	}
	out.close();
	in.close();
	
}
int main(){
	hardDrive();
	//pair<int, int> test=find();
	import("other.txt");

}
