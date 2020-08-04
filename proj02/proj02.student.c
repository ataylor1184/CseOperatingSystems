/************************** 
 * Alex Taylor
 * Project 02
***************************/
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <math.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
using std::string;

int main( int argc, char *argv[] ){
	int filecount = 0;
	int modify = 0;
	int truncate = 1;
	int buff = 256;
	string destptr;
	char *dest;
	char *sourceptr = argv[1];
	int filedest = -1;
	int temp = -1;
	
	for (int i = 2; argv[i] != NULL; i++){
		if (argv[i][0] != '-'){ //creates a variable for the source and destination files
			if (filecount == 0) {
				destptr = argv[i];
				char num_check = destptr[0];
				dest = argv[i];
				filecount++;
				int int_num_check = (int)num_check - 48; 
				if (int_num_check == buff){
					printf("No destination file is given");
					return 0;
				}
			}
			
			if (filecount > 1){
				printf("Too many file inputs (required 2) \n");
				return 0;
			}
		}
		
		if (argv[i][0] == '-'){ 
			if (strcmp(argv[i], "-b") == 0){
				if (argv[i+1] != NULL){				    //if there is something after -b...
					char number = *argv[i+1];
					if (isdigit(number)){			    //check if it is a number.
						buff = (int)number - 48;        //save number as integer 
						double(buff) = pow(2,buff);
					}	
				}
				if (argv[i+2] != NULL){			
					//if (-B number) is not the last option, increase I to continue parsing
					i++;
				}
			}
			
			else if (strcmp(argv[i], "-m") == 0){
				modify = 1;
			}
			else if (strcmp(argv[i], "-nm") == 0){
				modify = 0;
			}
			else if (strcmp(argv[i], "-t") == 0){
				truncate = 1;
			}
			else if (strcmp(argv[i], "-nt") == 0){
				truncate = 0;
			}
			else{
				printf("%s Is not a valid command!",argv[i]);
				return 0;
			}
		}
	}	
	int fd = open(sourceptr,O_RDONLY,S_IRWXU);
	
	if (fd == -1) {
		printf("Source file '%s' is not found \n",sourceptr);
		return 0;
	}
	else{
		filedest = open(dest,O_WRONLY,S_IRWXU);
		if (filedest == -1){		
				//if new file does not exist
				filedest = open(dest, O_WRONLY|O_CREAT|O_APPEND,S_IRWXU);
				char* buffer[buff];
				while (temp != 0) {
						ssize_t temp = read(fd,buffer,buff);
						write(filedest,buffer,temp);
						if (temp == 0){
							break;
						}
				close(fd);	
				}
		}
		else{										
			//if file does exist
			if (modify == 0){ 						
				//modify is off 
				printf("Modify option needs to be on to continue");
				return 0;
			}
			else{									
				//modify is on
				if (truncate == 1) {				//truncate is on 
					filedest = open(dest, O_WRONLY|O_TRUNC,S_IRWXU);
					char *buffer[buff];
					while (temp != 0) {
						ssize_t temp = read(fd,buffer,buff);
						write(filedest,buffer,temp);
						if (temp == 0){
							break;
						}
					}
				}
				else{								//truncate is off
					filedest = open(dest, O_WRONLY|O_APPEND,S_IRWXU);
					char *buffer[buff];
					while (temp != 0) {
						ssize_t temp = read(fd,buffer,buff);
						write(filedest,buffer,temp);
						if (temp == 0){
							break;
						}
					}
				}
			}
		}
	}
	return 0;	
}

