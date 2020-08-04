/*****************
*Alex Taylor
* proj08
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
unsigned startindex;

struct RAM {
	string ram[65536];
	unsigned index;
};

struct CACHE{
	string data[8];
};

void printram(struct RAM *ram, unsigned int end_index){
	cout << "Contents of Ram" << endl;
	ram->index = startindex;
	for (unsigned i = ram->index; i<end_index - 1;i++){
			cout << setfill('0') << setw(4) << hex << ram->index + (i*16) <<" "<<ram->ram[i]<<endl;
	};
};

void printcache(struct CACHE *cache){
	cout << "Contents of Cache" << endl;
	cout << "Index, V-Bit, M-Bit, Tag, Data" << endl;
	for (unsigned i = 0; i < 8; i++){
		cout <<"["<<i<<"]"<<"  " << cache->data[i] << endl;
	};
};

int read_ram_file(string file, struct RAM *ram, int debug){
	ifstream inpfile;
	inpfile.open(file);
	string line;
	if (!inpfile){
		return 0;
	}
	int line_count = 0;
	unsigned tempindex;
	unsigned index;
	
	while(getline(inpfile,line)){
		if (line_count == 0){
			stringstream ss;
			ss << hex << line;
			ss >> tempindex;
			ram->index = tempindex;
			startindex = tempindex;
		}
		else{
			ram->ram[tempindex-1] = line;
			
		}
		tempindex++;
		line_count++;
	}
	index = tempindex;
	if (debug == 1){
		printram(ram,index);
	}
	return index;
}

string inttohex(auto i){
	char buffer [33];
	sprintf(buffer,"%03x",i);
	return buffer;
}

int hextobin(string htag, string hline){
	string bin = "";
	for (unsigned i = 0; i < htag.length(); i++){
		if(i%4 == 0){
			bin.append(" ");
		}
		if (i == 0){
			switch (htag[i]){
				case '0': bin.append("0");break;
				case '1': bin.append("1");break;
			}
		}
		if (i != 0){
			switch (htag[i]){
				case '0': bin.append("0000");break;
				case '1': bin.append("0001");break;
				case '2': bin.append("0010");break;
				case '3': bin.append("0011");break;
				case '4': bin.append("0100");break;
				case '5': bin.append("0101");break;
				case '6': bin.append("0110");break;
				case '7': bin.append("0111");break;
				case '8': bin.append("1000");break;
				case '9': bin.append("1001");break;
				case 'a': bin.append("1010");break;
				case 'b': bin.append("1011");break;
				case 'c': bin.append("1100");break;
				case 'd': bin.append("1101");break;
				case 'e': bin.append("1110");break;
				case 'f': bin.append("1111");break;
			}
		}
	}
	for(unsigned j = 0; j < hline.length(); j++){
		switch (hline[j]){
			case '0': bin.append("000");break;
			case '1': bin.append("001");break;
			case '2': bin.append("010");break;
			case '3': bin.append("011");break;
			case '4': bin.append("100");break;
			case '5': bin.append("101");break;
			case '6': bin.append("110");break;
			case '7': bin.append("111");break;
			
		}
	}
	bin.append("0000");

	int value = (int)strtol(bin.c_str(),NULL,2);
	return value;
}

int read_refs_file(struct RAM *ram, string file, struct CACHE *cache,int debug){
	ifstream inpfile;
	inpfile.open(file);
	string line;
	if (!inpfile){
		return 0;
	}
	if (debug == 1){
		printcache(cache);
	}
	cout << endl;
	cout << "Memory Reference" << endl;
	
	while (getline(inpfile,line)){
		string result;
		char temp[516];
		strcpy(temp,line.c_str());
		unsigned tempindex;
		char *token = strtok(temp," ");
		string address(token);
		token = strtok(NULL," ");
		string operation = token;
		stringstream ss;
		token = strtok(NULL," ");
	
		ss << hex << address;
		ss >> tempindex;

		ram->index = tempindex;
		
		unsigned tag = tempindex>>7;
		unsigned cache_line = ((tempindex>>4)<<29)>>29;
		unsigned offset = (tempindex<<28)>>28;
		unsigned entryindex = offset;

		string address_cacheline;
		string buf;
		vector<string> cache_tokens;
		vector<string> ram_tokens;
		vector<string> dvalue;
		string cache_entry;
		char hitmiss;
		string datavalue;
	
		
		//Process a ref entry.
		for (unsigned i = 0; i < 8; i ++){			//iterate over cache
			if (i == cache_line){					//check correct slot
				stringstream sscache(cache->data[i]);
				while (sscache >> buf){
					cache_tokens.push_back(buf);
				}
				int bindex = hextobin(cache_tokens[2],to_string(cache_line));
				string t = inttohex(tag);
				if (cache_tokens[0] == "0"){		//v = 0 , miss
					hitmiss = 'M';
					cache_entry.append("1"); 		
					if (operation == "W"){			//miss write command
						cache_entry.append(" 1 ");	
						cache_entry += (t);			
						cache_entry.append(" ");
						stringstream ssram(ram->ram[ram->index - offset]);		
						while (ssram >> buf){
							ram_tokens.push_back(buf);
						}
						
						
						while (token != NULL){
							ram_tokens[entryindex] = token;
							entryindex++;
							token = strtok(NULL," ");
						}
						for (auto i = 0; i < 16; i++){
							cache_entry.append(ram_tokens[i]);
							cache_entry.append(" ");
						}
					}
					else{							//miss, read command
						cache_entry.append(" 0 ");	
						cache_entry += (t);			
						cache_entry.append(" ");
						cache_entry.append(ram->ram[tempindex]);
					}
				}
				else{ 								//v = 1
					if (cache_tokens[2] == t){ 		//hit, tags match, v = 1
						hitmiss = 'H';
						if (operation == "W"){		//hit, tags match, write command
							cache_entry.append(" 1 ");	
							cache_entry += (t);			
							cache_entry.append(" ");
							while (token != NULL){
								ram_tokens[entryindex] = token;
								entryindex++;
								token = strtok(NULL," ");
							}
							for (auto i = 0; i < 16; i++){
								cache_entry.append(ram_tokens[i]);
								cache_entry.append(" ");
							}	
							
						}
						else{						//hit, tags match, read command
							cache_entry.append(" 0 ");	
							cache_entry += (t);			
							cache_entry.append(" ");
							cache_entry.append(ram->ram[tempindex]);
						}
					}
					else{							//miss, tags mismatch, v =1 1
						hitmiss = 'M';
						if (cache_tokens[1] == "1"){//write back to ram
							string temp3 = "";
							temp3.append(cache_tokens[3]);
							for (auto q = 4; q < 19; q++){
								temp3.append(" ");
								temp3.append(cache_tokens[q]);
							}
							ram->ram[bindex] = temp3;					//adding to current index, not the index shown in cache line
						}
						cache_entry.append("1"); 		
						if (operation == "W"){		//hit, tags dont match, write		
							cache_entry.append(" 1 ");	
							cache_entry += (t);			
							cache_entry.append(" ");
							while (token != NULL){
								ram_tokens[entryindex] = token;
								entryindex++;
								token = strtok(NULL," ");
							}
							for (auto i = 0; i < 16; i++){
								cache_entry.append(ram_tokens[i]);
								cache_entry.append(" ");
							}
						}
						else{							//hit, tags dont match, read
							cache_entry.append(" 0 ");	
							cache_entry += (t);			
							cache_entry.append(" ");
							cache_entry.append(ram->ram[tempindex]);
						}
					}
				}
				cache->data[cache_line] = cache_entry;
			}
		}
		
		stringstream sscache(cache->data[cache_line]);		//getting data value
		while (sscache >> buf){
			dvalue.push_back(buf);
		}
		for(auto j = offset+3; j < offset + 7; j++){
			datavalue.append(dvalue[j]);
			datavalue.append(" ");
		}
		
		cout << address<<" "<< operation<<" ";
		cout << hex<< cache_line <<" "<< setfill('0')<< setw(3) << tag <<" ";
		cout << offset << " " << hitmiss << " " << datavalue << endl;
		
		datavalue = "";
		cache_entry = "";
		
		if (debug == 1){
			cout << endl;
			printcache(cache);
			cout << endl;
		}
	}
	return -1;
}


int main(int argc, char** argv){
	struct RAM main_ram;
	struct CACHE main_cache;
	string filler = "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";
	string fillerc = "0 0 000 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";
	size_t n = sizeof(main_ram)/sizeof(main_ram.ram[0]);
	for(unsigned i = 0; i<n; i++){
		main_ram.ram[i] = filler;
	}
	
	size_t q = sizeof(main_cache)/sizeof(main_cache.data[0]);
	for(unsigned i = 0; i<q; i++){
		main_cache.data[i] = fillerc;
	}
	
	int debug_switch = 0;
	int ram_switch = 0;
	int ref_switch = 0;
	string ram_file;
	string ref_file;
	
	for (auto i = 1; i < argc; i++){
		int valid = 0;
		if (string(argv[i]) == "-ram"){
			i++;
			if (argv[i]){
				ram_file = argv[i];
				ram_switch = 1;
				valid = 1;
			}
		}
		if (string(argv[i]) == "-refs"){
			i++;
			if (argv[i]){
				ref_file = argv[i];
				ref_switch = 1;
				valid = 1;
			}
		} 
		if (string(argv[i]) == "-debug"){
			debug_switch = 1;
			valid = 1;
		}
		else if (valid == 0){
			cout << string(argv[i]) << endl;
			cout << "Valid options are (-ram, -ref, debug)" << endl;
			cout << ("ref and ram must be followed by valid file name") << endl;
			return 0;
		
		} 
	}
	
	unsigned index = read_ram_file(ram_file,&main_ram, debug_switch);
	int valid_file = read_refs_file(&main_ram, ref_file, &main_cache, debug_switch);

	if (ram_switch == 1 and index != 0 ){
		cout << endl;
		printram(&main_ram,index);
	}
	
	if (ram_switch == 0){
		cout << endl;
		cout << "No ram subset was selected" << endl;
	}
	
	if (valid_file == 0 and ref_switch == 1){
		cout << "Invalid ref file" << endl;
	}
	
	cout << endl;
	printcache(&main_cache);
	
	return 0;
}
