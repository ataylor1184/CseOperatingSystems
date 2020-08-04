/* 
 * File:   main.cpp
 * Author: Alex
 *
 * Created on March 3, 2019, 9:48 PM
 */

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

using namespace std;


struct cache{ 
    char* memc[8];  
    
};

struct cache CACHE;

struct ram{
	char* mem[65536]; 

};
struct ram RAM;
void testprint(int address){
	printf("\n %d ",address);
	for (int j = 0; j < 1; j++){
		for (int i = 0; i < 16; i++){
			printf("%s \n",RAM.mem[address + i]);
		}
		address += 16;
	}
	printf("\n");
}

char* hextobin(char* hexstring){
	char* binstring;
	strcpy(binstring,"");
	char zero[] = "0000";
	char one[] = "0001";
	char two[] = "0010";
	char three[] = "0011";
	char four[] = "0100";
	char five[] = "0101";
	char six[] = "0110";
	char seven[] = "0111";
	char eight[] = "1000";
	char nine[] = "1001";
	char a[] = "1010";
	char b[] = "1011";
	char c[] = "1100";
	char d[] = "1101";
	char e[] = "1110";
	char f[] = "1111";

	for(size_t i = 0; i < strlen(hexstring); i++){
		if (hexstring[i]-48 == 0){
			strcat(binstring, zero);
		}
		if (hexstring[i]-48 == 1){
			strcat(binstring, one);
		}
		if (hexstring[i]-48 == 2){
			strcat(binstring, two);
		}
		if (hexstring[i]-48 == 3){
			strcat(binstring, three);
		}
		if (hexstring[i]-48 == 4){
			strcat(binstring, four);
		}
		if (hexstring[i]-48 == 5){
			strcat(binstring, five);
		}
		if (hexstring[i]-48 == 6){
			strcat(binstring, six);
		}
		if (hexstring[i]-48 == 7){
			strcat(binstring, seven);
		}
		if (hexstring[i]-48 == 8){
			strcat(binstring, eight);
		}
		if (hexstring[i]-48 == 9){
			strcat(binstring, nine);
		}
		if (hexstring[i]-48 == 49){
			strcat(binstring, a);
		}
		if (hexstring[i]-48 == 50){
			strcat(binstring, b);
		}
		if (hexstring[i]-48 == 51){
			strcat(binstring, c);
		}
		if (hexstring[i]-48 == 52){
			strcat(binstring, d);
		}	
		if (hexstring[i]-48 == 53){
			strcat(binstring, e);
		}
		if (hexstring[i]-48 == 54){
			strcat(binstring, f);
		}	
	}
	
	return binstring;
}

int bintodec(char* in){
	int temp = 0;
	int dec = 0;
	int expo = 1;
	int num;
	for(int i= strlen(in); i >= 0; i--){
		if (in[i]){
			num = in[i] - 48;
			dec += num * expo;
			//printf("expo: %d num: %d dec: %d\n",expo,num,dec);
			expo = expo *2;
		}
	}
	//printf("Inp: %s dec: %d\n",in, dec);	
	return dec;
}
int main(int argc, char** argv) {
	//ram variables
	FILE* fp_ram;
	ssize_t read_ram;
	size_t len_ram = 0;
	char* line_ram = NULL;
	char* ram_input;
	int address;
	char* temp1;
	int line_count; 
	int start_address;
	int ram_called = 0;
	//ref variables
	char* ref_input;
	ssize_t read_ref;
	char* line_ref = NULL;
	char* temp2;
	size_t len_ref =0;
	FILE* fp_ref; 
	int start_ref;
	char read_write[2];
	int ref_address;
	char* bin_string;
	char* hex_address;
	int c_out;
	int t_out;
	int o_out;

	char* input;
	char start[] = "00";
	for(int i = 0; i < 65536; i++){
		RAM.mem[i] = start;
	}
	/*for(int k = 0; k < 8; k++){	//this gives me a segfault?
		printf("k: %d \n");
		CACHE.memc[k] = start;
	}*/

	for (int i = 1; argv[i] != NULL; i++){
		input = argv[i];
		//printf("%d: %s \n",i,input);
		if (strcmp(input, "-ram") == 0) {
			if (ram_called == 1){
				
				printf("-ram option already executed with file %p \n", fp_ram);
			}
			i++;
			fp_ram = fopen(argv[i], "r");  

			if (fp_ram){
				ram_called = 1;
				read_ram = getline(&line_ram, &len_ram, fp_ram);
				if (read_ram != -1) {
					ram_input = strtok(line_ram, " \n");
					
					start_address = strtol(ram_input,&temp1,16);
					address = start_address;
					ram_input = strtok(NULL," ");
				}
				while(read_ram != -1){ //loop over lines of file
					while (ram_input != NULL){ //loop over content of lines	
						RAM.mem[address] = ram_input;
						address += 1;
						ram_input = strtok(NULL," \n");
					}
					read_ram = getline(&line_ram, &len_ram, fp_ram);
				}
				fclose(fp_ram);

			}
			else{
				printf("Failed to open %s as a Ram File \n", argv[i]);
			}
		}
		
		//testprint(4768);

		if (strcmp(input, "-refs") == 0) {
			i++;
			fp_ref = fopen(argv[i], "r");  

			if (fp_ref){
				read_ref = getline(&line_ref, &len_ref, fp_ref);	
				while(read_ref != -1){ 
					ref_input = strtok(line_ref, " \n");					//first token of line
					hex_address = ref_input;
					start_ref = strtol(ref_input,&temp2,16);				//save address
					bin_string = hextobin(ref_input);
					//printf("bin_string: %s \n", bin_string);
					ref_address = start_ref;
					ref_input = strtok(NULL," \n");							//get second token
					if (strcmp(ref_input, "R") == 0){
						strcpy(read_write, "R");
					}
					if (strcmp(ref_input, "W") == 0){
						strcpy(read_write, "W");
						ref_input = strtok(NULL," \n");
						while (ref_input != NULL){							//loop over contents to write
							//RAM.mem[ref_address] = ref_input;
							ref_address += 1;
							ref_input = strtok(NULL," \n");
						}
					}
					
		
					//printf("start ref: %d \n",start_ref);

					char cline[20];
					char tag[20];
					char offset[20];
		
					for(int j = 0; j < strlen(bin_string); j++){
						int temp = bin_string[j] - 48;
						char* value;
						sprintf(value,"%d", temp);
						if(j<9){
							strcat(tag,value);
						}
						else if(j>=9 && j<12){
							strcat(cline,value);
							
						}
						else if(j>=12 && j < 17){
							strcat(offset,value);
						}
					}

					//printf("_______________________\n");
					//printf("Cache Line: %s \n",cline);
					//printf("tag bits: %s \n",tag);
					//printf("offset: %s \n",offset);
					c_out = bintodec(cline);
					t_out = bintodec(tag);
					o_out = bintodec(offset);
					strcpy(cline,"");
					strcpy(tag,"");
					strcpy(offset,"");
					
					printf("%s %s %d %d %d \n",hex_address,read_write,c_out,t_out,o_out);
					read_ref = getline(&line_ref, &len_ref, fp_ref);	
				}
				fclose(fp_ref);
			}
			else{
				printf("Failed to open %s as a Ref File \n", argv[i]);
			}
		}
		//testprint(4768);
		if (strcmp(input, "-debug") == 0) {
			printf("Executing -debug \n");	
		}
			
	}
}

