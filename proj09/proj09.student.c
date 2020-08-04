/*****************
*Alex Taylor
* proj09
* 
************/


#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <bitset>
#include <sstream>


using namespace std;

int rcount = 0;
int wcount = 0;
int refcount = 0;

struct PAGE{
	vector<string> contents[8192];
	int vbit = 0;
	int rbit = 0;
	int mbit = 0;
	int framenum = 000;
};

struct PAGETABLE{
	struct PAGE pages[8];
};

void printtable(struct PAGETABLE *pagetable){
	cout << "Contents of Page Table" << endl;
	cout << "Index, V-Bit, R-Bit, M-Bit, Frame Number" << endl;
	for (unsigned i = 0; i < 8; i++){
		cout <<"["<<i<<"]"<<" " << pagetable->pages[i].vbit << " "<< pagetable->pages[i].rbit;
		cout << " "<< pagetable->pages[i].mbit << " "<< setfill('0') << setw(3)<< pagetable->pages[i].framenum << endl;
	};
};

int read_refs_file(string file, struct PAGETABLE *pagetable, int debug){
	ifstream inpfile;
	inpfile.open(file);
	string line;
	
	if (!inpfile){
		return 0;
	}
	if (debug == 1){
		printtable(pagetable);
	}
	cout << endl;
	cout << "Memory Reference" << endl;
	
	while (getline(inpfile,line)){
		string result;
		char temp[516];
		strcpy(temp,line.c_str());
		unsigned tempindex;
		char *token = strtok(temp," ");
		string physicaladdress(token);
		token = strtok(NULL," ");
		string operation = token;
		stringstream ss;

		ss << hex << physicaladdress;
		ss >> tempindex;
	
		if (operation == "R"){
			rcount++;
		}
		else{
			wcount++;
		}
		
		unsigned pagenumber = tempindex>>13;
		unsigned pageoffset = (tempindex & 0x1FFF);
		
		cout << setfill('0') << setw(4) << physicaladdress << " ";
		cout << pagenumber << " ";
		cout << hex << setfill('0') << setw(4) << pageoffset << " "; 
		cout << operation << endl;
		
		if (debug == 1){
			cout << endl;
			printtable(pagetable);
			cout << endl;
		}
	}
	refcount = wcount + rcount;
	return -1;
}


int main(int argc, char** argv){
	struct PAGETABLE main_pagetable;
	struct PAGE blank_page;
	
	for (auto i = 0; i < 8; i++){
		main_pagetable.pages[i] = blank_page;
	}
	
	int debug_switch = 0;
	int ref_switch = 0;
	string ref_file;
	
	for (auto i = 1; i < argc; i++){
		int valid = 0;
		if (string(argv[i]) == "-refs"){
			ref_file = argv[i+1];
			i++;
			ref_switch = 1;
			valid = 1;
			continue;
		} 
		if (string(argv[i]) == "-debug"){
			debug_switch = 1;
			valid = 1;
			continue;
		}
		else if (valid == 0){
			cout << string(argv[i]) << endl;
			cout << "Valid options are (-ref, -debug)" << endl;
			cout << "ref must be followed by valid file name" << endl;
			return 0;
		
		} 
	}
	
	int valid_file = read_refs_file(ref_file, &main_pagetable, debug_switch);

	if (valid_file == 0 and ref_switch == 1){
		cout << "Invalid ref file" << endl;
	}
	cout << endl;
	cout << dec << "Total Memory References: %d" << refcount << endl;
	cout << "Total Read Operations:   %d" << rcount << endl;
	cout << "Total Write Operations:  %d" << wcount << endl;
	cout << endl;
	printtable(&main_pagetable);
	return 0;
}
