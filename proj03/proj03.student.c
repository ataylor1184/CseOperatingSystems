/************************** 
 * Alex Taylor
 * Mini Shell Program
 * Project 03
***************************/
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
using namespace std;

char* String_Cleaner(char* str){
	unsigned int length, i , j;
	char *original_input;
	i=0;
	j=0;
	length = strlen(str);						//looping through and leaving 1 space or tab between words
	original_input = (char*)malloc(length+1);
	while (*(str+i) != '\0'){
		if (*(str+i) == ' ' || *(str+i) == '\t'){
			int temp = i+1;
			while ((*(str+temp) == ' ' || *(str+temp) == '\t') && *(str+temp) != '\0'){
				if (*(str+temp) == ' ' || *(str+temp) == '\t'){
					i++;
				}
				temp++;
			}
		}
		*(original_input+j) = *(str+i);
		i++;
		j++;
		
	}
	*(original_input+j) = '\0';
										//converting the tabs between words to single spaces
	for(i=0; i < strlen(original_input); i++){
		if(original_input[i] == '\t'){
			(original_input[i] = ' ');
		}
	}
	
	return original_input;  //returns original input string with single spaces seperating tokens
}

int main( int argc, char *argv[], char **envp){
	int linecount = 1;
	char *username = getlogin();
	char command[512];  //max size of input is 128 but would segfault if something bigger is given
	int end = 0;
	char* token;
	int progress;
	char* othername;

	while (end == 0) {
		printf("%d %s: ",linecount,username);
		progress =0;
		scanf(" %[^\n]", command);				//collect user input
		if (strlen(command) > 128) {			//check for input size
			printf("Error: Input is too large (128 char max)\n");
		}
		else{
			token = String_Cleaner(command);
		}
		token = strtok(token, " ");				//break input into tokens
		while (token != NULL && progress == 0){
			
			if (token == NULL){
				printf("%s: Command not found.\n", token);
				break;
			}
			
			if (strcmp(token, "exit") == 0){		//beginning of command processing
				progress = -1;
				break;
			}
		
			else if (strcmp(token, "help") == 0){
				printf("--Valid Commands--\n\n");
				printf("Typing   'exit'    Will exit the shell.\n");
				printf("Typing   'help'    Will display all available commands.\n");
				printf("Typing   'now'     Will display the current time and date.\n");
				printf("Typing   'env'     Will display current environment variables.\n");
				printf("Typing   'path'    Will display your current search path.\n");
				printf("Typing   'cwd' 	   Will display the absolute pathname of your current directory.\n");
				printf("Typing   'cd DIR'  Will reset the current directory to be DIR.\n");
				printf("Typing   'cd ~%s'  Will set your current directory to your home directory.\n\n",username);
				progress = 1;
			}
			
			else if (strcmp(token, "now") == 0){
				time_t timer;
				char buffer[50];
				struct tm* tm_info;				//displays time in readable format
				time(&timer);
				tm_info = localtime(&timer);
				strftime(buffer,50, "Current Date: %Y-%m-%d Current Time: %H:%M:%S", tm_info);
				puts(buffer);
				progress = 1;
			}
			
			else if (strcmp(token, "env") == 0){
				for (char **env = envp; *env !=0; env++){  //displays all current enviroment variables
					char *CurrentEnv = *env;
					printf("%s\n", CurrentEnv);	
				}
				progress = 1;
			}
			
			else if (strcmp(token, "path") == 0){
					char* path = getenv("PATH");  //displays current available paths
					path = strtok(path, ":");
					while (path != NULL){
						printf("Path: %s\n",path);
						path = strtok(NULL,":");
					}
					progress = 1;
			}
			
			else if (strcmp(token, "cwd") == 0){
				char* DIR = get_current_dir_name();
				if (DIR == NULL){
						printf("Unable to find file");
					}
					else{
						printf("%s \n", DIR);
						free(DIR);
					}
				progress = 1;
			}
			
			else if (strcmp(token, "cd") == 0){  //changes directory
				token = strtok(NULL," ");
				int working = 2;
				const char* userpath = "/user/";
				char* home_dir;
				char* new_dir;
				new_dir = (char*)malloc(256);			//creates a home directory with current user's name
				home_dir = (char*)malloc(strlen(username)+6);
				strcpy(home_dir,userpath);
				strcat(home_dir,username);

				if (token != NULL){
					if (token[0] == '~'){		//processing 'cd ~USERNAME' commands
						char* temp = token;
						othername = temp +1;
						strcpy(new_dir,"/user/");
						strcat(new_dir,othername);
						working = chdir(new_dir);

						if (working != 0){
							printf("%s is not a valid directory\n", new_dir);
						}
						else{
							progress = 1;
						}
					}
					else{
						char* temp = token;			//processing 'cd DIR' commands
						strcpy(new_dir,temp);
						printf("Trying to move to: %s\n", new_dir);
						working = chdir(new_dir);
						
						if (working != 0){
							printf("%s is not a valid directory\n", new_dir);
						}
						else{
							progress = 1;
						}
					}
				free(home_dir);
				free(new_dir);
				}
				else{
					chdir(home_dir);				//if only cd is entered, move to home directory
					progress = 1;
				}
			}
		token = strtok(NULL," ");				//update token to next input
		}
	if (progress == 0){
		printf("Invalid Command.\n");
	}		
	else if (progress == -1){
		break;
	}	
	linecount++;						//update line count
	}

	return 0;	

}
