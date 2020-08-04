/*****************
*Alex Taylor
* proj10
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
#include <bitset>
#include <list>
#include <iterator>

using namespace std;

struct PAGE{
	vector<string> contents[8192];
	int vbit = 0;
	int rbit = 0;
	int mbit = 0;
	int framenum = 000;
};

struct PAGETABLE{
	struct PAGE pages[8];
	int rcount = 0;
	int wcount = 0;
	int refcount = 0;
	string algorithm;
	int framecount = 0;
	int pfcount = 0;
	int wbcount = 0;
};

void printtable(struct PAGETABLE *pagetable){
	cout << "Contents of Page Table" << endl;
	cout << "Index, V-Bit, R-Bit, M-Bit, Frame Number" << endl;
	for (unsigned i = 0; i < 8; i++){
		cout <<"["<<i<<"]"<<" " << pagetable->pages[i].vbit << " "<< pagetable->pages[i].rbit;
		cout << " "<< pagetable->pages[i].mbit << " "<< setfill('0') << hex <<  setw(3)<< pagetable->pages[i].framenum << dec << endl;
	};
	cout << endl;
};

void printpage(struct PAGE *page){
	cout << endl;
	cout << hex << "V: " << page->vbit << endl;
	cout << "R: " << page->rbit << endl;
	cout << "M: " << page->mbit << endl;
	cout << "Frame: " << page->framenum << dec <<  endl;
}

void printlist(list <int> g) 
{ 
	cout << "LIST: "<<hex;
    list <int> :: iterator it; 
    for(it = g.begin(); it != g.end(); ++it) 
        cout << ' ' << *it; 
    cout <<dec << endl; 
} 

int read_refs_file(string file, struct PAGETABLE *pagetable, int debug){
	ifstream inpfile;
	inpfile.open(file);
	string line;
	unsigned tempindex;
	stringstream ss;
	stringstream s2;
	unsigned framecount;
	unsigned firstframe = 0;
	int freecount;
	int tempdata = 1;
	list <int> fifo;
	list <int> lru;

	if (!inpfile){
		return 0;
	}
	if (debug == 1){
		printtable(pagetable);
	}
			
	getline(inpfile,line);									//Fifo or LRU
	char temp[516];
	strcpy (temp,line.c_str());
	string removetype(temp);
	pagetable->algorithm = removetype;
	//cout << "removetype: " << removetype << endl;

	getline(inpfile,line);									//Frame Count
	strcpy (temp,line.c_str());
	framecount = stoi(temp,NULL,0);
	pagetable->framecount = framecount;
	//cout << "framecount: " << framecount << endl;
	
	getline(inpfile,line);									//Frame num to start at
	strcpy (temp,line.c_str());
	firstframe = stoi(temp,NULL,16);
	//cout << hex << "firstframe: " << firstframe << dec << endl;

	unsigned freeframelist[framecount];						//initialize free frame list
	for (unsigned i = 0; i < framecount; i++){
		freeframelist[i] = firstframe + i;
	}
	freecount = framecount;

	cout << "Memory Reference" << endl;
	while (getline(inpfile,line)){
		string writeback = " ";
		string pagefault = " ";
		unsigned nextframe = 65535;
		char temp[516];
		strcpy(temp,line.c_str());
		
		char *token = strtok(temp," ");
		string logicaladdress(token);					//logical address
		
		token = strtok(NULL," ");
		string operation = token;						//operation

		tempindex = stoi(logicaladdress, NULL,16);
		
		struct PAGE newpage;							//making page to be added
		newpage.vbit = 1;
		newpage.rbit = 1;
		
		if (operation == "R"){							//op counts
			pagetable->rcount++;
		}
		else{
			pagetable->wcount++;
			newpage.mbit = 1;
		}
		
		unsigned pagenumber = tempindex>>13;			//offset and page num
		unsigned pageoffset = (tempindex & 0x1FFF);

		if (pagetable->pages[pagenumber].vbit == 1){				//v=1
			newpage.framenum = pagetable->pages[pagenumber].framenum;		
			if (pagetable->pages[pagenumber].mbit == 1){			//m=1,v=1
				newpage.mbit = 1;
			}
		}
		else{														//v=0
			pagetable->pfcount++;
			pagefault = "F";
			if (freecount > 0){
				for (unsigned i = 0; i < framecount; i++){
					if (freeframelist[i] != 0){					//keeps count of number of unused frames
						if (freeframelist[i] < nextframe){		//finds lowest frame in list to use
							nextframe = freeframelist[i];
							freeframelist[i] = 0;
							freecount--;
						}
					}
				}
				newpage.framenum = nextframe;
			}
			else{
				if (pagetable->algorithm == "LRU"){
					newpage.framenum = lru.back();
					lru.pop_back();
				}
				else{
					newpage.framenum = *fifo.begin();
					fifo.pop_front();
				}	
				for (unsigned i = 0; i < 8; i++){
					if(pagetable->pages[i].framenum == newpage.framenum){
						if (pagetable->pages[i].mbit == 1 && pagetable->pages[i].vbit == 1){
							pagetable->wbcount++;
							writeback = "B";
						}
						pagetable->pages[i].vbit = 0;
					}
				}
			}
		}
		pagetable->pages[pagenumber] = newpage;
		fifo.push_back(nextframe);
		lru.remove(newpage.framenum);
		lru.push_front(newpage.framenum);
		unsigned physicaladdress = (newpage.framenum << 13) | pageoffset;

	
		cout << setfill('0') << setw(4) << logicaladdress << " ";		//outputs
		cout << operation << " "<< pagenumber << " ";
		cout << hex << setfill('0') << setw(4) << pageoffset << " "; 
		cout << pagefault << " "<< writeback << " ";
		cout << hex << setfill('0') << setw(6) <<  physicaladdress << dec;
		cout << endl;
		
		if (debug == 1){
			cout << endl;
			printtable(pagetable);
			cout << endl;
		}
	}
	pagetable->refcount = pagetable->wcount + pagetable->rcount;
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
	int valid = 0;
	
	for (auto i = 1; i < argc; i++){
		valid = 0;
		if (string(argv[i]) == "-refs"){
			if (argv[i+1] == NULL){
				cout << "-refs must be followed by valid file name" << endl;
				valid = 0;
				continue;
			}
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
			return 0;
		} 
	}
	
	int valid_file = read_refs_file(ref_file, &main_pagetable, debug_switch);

	if (valid_file == 0 and ref_switch == 1){
		cout << "Invalid ref file" << endl;
	}
	if (valid != 0){
		cout << endl;
		cout << "Total Memory References: " << dec << main_pagetable.refcount << endl;
		cout << "Total Read Operations:   " << main_pagetable.rcount << endl;
		cout << "Total Write Operations:  " << main_pagetable.wcount << endl;
		cout << "Total Page Faults:       " << main_pagetable.pfcount << endl;
		cout << "Total Write Backs:       " << main_pagetable.wbcount << endl;
		cout << endl;
		printtable(&main_pagetable);
	}
	return 0;
}
