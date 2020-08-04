/************************** 
 * Alex Taylor
 * Mini Shell Program
 * Project 04
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
#include <pthread.h>

char* String_Cleaner(char* str){
	//removes blanks, tabs and newline chars from a line
	//returns char array of 'words' seperated by single blank spaces.
	unsigned int length, i , j;
	char *original_input;
	i=0;
	j=0;
	length = strlen(str);						//looping through and leaving 1 space or tab between words
	original_input = (char*)malloc(length+1);
	while (*(str+i) != '\0'){
		if (*(str+i) == ' ' || *(str+i) == '\t'){
			int temp = i+1;
			while ((*(str+temp) == ' ' || *(str+temp) == '\t' || *(str+temp) == '\n' ) && *(str+temp) != '\0'){
				if (*(str+temp) == ' ' || *(str+temp) == '\t' || *(str+temp) == '\n'){
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
		if(original_input[i] == '\t' || original_input[i] == '\n'){
			(original_input[i] = ' ');
		}
	}
	return original_input;  //returns original input string with single spaces seperating tokens
}

void* threadmaker(void *comm){
	//executes a system call for a external command found in a file.
	system((char*)comm);
	pthread_exit(NULL);
}

int main(int argc, char* command_file = NULL, int fromfile = 0){
	/***************************************************************************************************************
	 * A note on input for this project.
	 * When typed from the user the program will pull the command out from a list of tokens such as "give time now"
	 * Will execute as if 'now' was the command.
	 * When using input from a file the program will only run commands if they are the first token.
	 * 
	 * I think its more user friendly this way.
	 * *************************************************************************************************************/
	int linecount = 1;
	char *username = getlogin();
	char command[512];  //max size of input is 128 but would segfault if something bigger is given
	int end = 0;
	char* token;
	int progress = 0;
	char* othername;
	int background = 0;
	while (end == 0) {
		if (fromfile == 1){											//when reading from file, fromfile = 1
			strcpy(command,command_file);
			char last_letter = command[(strlen(command)-1)];
			if (last_letter == '&'){ 									//check if last char of line from a file input is a ampersand
				background = 1;
			}
		}
		
		if (fromfile != 0 && fromfile != 1){							//get user input if not reading from file
			printf("[%d %s]",linecount,username);
			scanf(" %[^\n]",command);
		}
		progress = 0;
		
		if (strlen(command) > 128) {									//check for input size
			printf("Error: Input is too large (128 char max)\n");
		}

		else{
			token = String_Cleaner(command);  							//turns input into 'sentence' format
		}
		token = strtok(token, " \n");									//break input into tokens
		while (token != NULL && progress == 0){							//while there are still commands to process
			if (token == NULL){
				printf("%s: Command not found.\n", token);
				break;
			}
			
			if (strcmp(token, "exit") == 0){							//exits the mini shell
				end = -1;
				return -1;
			}

			else if (strcmp(token, "help") == 0){
				printf("--Valid Commands--\n\n");
				printf("Typing   'exit'    Will exit the shell.\n");
				printf("Typing   'help'    Will display all available commands.\n");
				printf("Typing   'now'     Will display the current time and date.\n");
				printf("Typing   'env'     Will display current environment variables.\n");
				printf("Typing   'path'    Will display your current search path.\n");
				printf("Typing   'cwd' 	   Will display the absolute pathname of your current directory.\n");
				printf("Typing   'cd' 	   Can be used to manage your current working directory.\n");
				printf("Typing   'set'     Can be used to manage your environment variables.\n");
				printf("Typing   'read FILENAME'    Can be used to import commands from a file.\n");

				progress = 1;
			}
			
			else if (strcmp(token, "now") == 0){
				time_t timer;
				char buffer[50];
				struct tm* tm_info;										//displays time in readable format
				time(&timer);
				tm_info = localtime(&timer);
				strftime(buffer,50, "Current Date: %Y-%m-%d Current Time: %H:%M:%S", tm_info);
				puts(buffer);
				progress = 1;
			}
			
			else if (strcmp(token, "env") == 0){
				extern char **environ;									//displays environment variables line by line
				for (int i = 0; environ[i] != NULL;){
					printf("%s\n", environ[i++]);	
				}
				progress = 1;
			}
			
			else if (strcmp(token, "path") == 0){
					char* path = getenv("PATH");  						//displays current available paths
					path = strtok(path, ":");
					while (path != NULL){
						printf("Path: %s\n",path);
						path = strtok(NULL,":");
					}
					progress = 1;
			}
			
			else if (strcmp(token, "cwd") == 0){						//displays the current working directory of the user.
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
			
			else if (strcmp(token, "cd") == 0){  						//changes directory
				token = strtok(NULL," ");
				int working = 2;
				const char* userpath = "/user/";
				char* home_dir;
				char* new_dir;
				new_dir = (char*)malloc(256);							//creates a home directory with current user's name
				home_dir = (char*)malloc(strlen(username)+6);
				strcpy(home_dir,userpath);
				strcat(home_dir,username);
				if (token != NULL){
					if (token[0] == '~'){								//processing 'cd ~USERNAME' commands
						char* temp = token;
						othername = temp +1;
						strcpy(new_dir,"/user/");
						strcat(new_dir,othername);
						working = chdir(new_dir);
						
						if (working != 0){
							printf("%s is not a valid directory\n", new_dir);
						}
						else{
							setenv("PWD",new_dir,1);
							progress = 1;
						}
					}
					
					else{
						char* temp = token;								//processing 'cd DIR' commands
						strcpy(new_dir,temp);
						working = chdir(new_dir);
						if (working != 0){
							if (fromfile == 1){
								progress = 1;
							}
							printf("%s is not a valid directory\n", new_dir);
						}
						else{
							char* DIR = get_current_dir_name();
							setenv("PWD",DIR,1);						//setenv updates directory in environment variables
							free(DIR);
							progress = 1;
						}
					}
				free(home_dir);
				free(new_dir);
				}
				else{
					chdir(home_dir);									//if only cd is entered, move to home directory
					setenv("PWD",home_dir,1);
					progress = 1;
				}
			}
		
			else if (strcmp(token,"read") == 0){						//read 1 line from file at a time
				token = strtok(NULL," "); 								//update token
				char* filename = token;
				if (filename == NULL){
					printf("A filename is required to use the read command, type help for details.\n");
					progress = 1;
				}
				else{ 													//the user entered the command properly
					FILE* fp;
					char* line = NULL;
					size_t len = 0;
					ssize_t read;
					char* DIR_Read = (char*)malloc(256);			
					strcpy(DIR_Read,get_current_dir_name());
					strcat(DIR_Read,"/");
					strcat(DIR_Read,filename);
					fp = fopen(DIR_Read,"r");
					free(DIR_Read);
					if (fp == NULL){									//unable to open file for whatever reason
						printf("Unable to open file, type 'cwd' to check your directory, or check file permissions\n");
					}
					else{
						fromfile = 1;
						read = getline(&line, &len, fp);
						while(read != -1){								//iterating through file line at a time.
							printf("%s",line);
							end = main(1,line,1);
							if (end == -1){								//only occurs when exit is found in a file
								break;
							}
							read = getline(&line, &len, fp);
						}
						fclose(fp);
						fromfile = 2;
						if(line != NULL){
							free(line);
						}
						progress = 1;
					}	
				}
			}	
			
			else if (strcmp(token, "set") == 0){						//sets environment variables to user input
				token = strtok(NULL," ");								//setting a existing variable will overight its value
				const char* VAR = token;
				token = strtok(NULL," ");
				const char* VALUE = token;
				if (VALUE == NULL) {
					int Err =  unsetenv(VAR);
					progress = 1;
					if (Err != 0) {										
						printf("Unable to delete Enviroment Variable %s", VAR);
					}
				}
				else if (VAR == NULL){
					printf("Invalid Command, set requires a variable and/or value\n");
				}
				else{
					progress = 1;
					setenv(VAR,VALUE,1);
				}
			}

			else{
				

				if (fromfile == 1){
					progress =1;
					pthread_t thread_id;								//create thread to run external command
					pthread_create(&thread_id, NULL, threadmaker, command_file);
					if (background != 1){ 								//do not call join if you want command to run in background
						pthread_join(thread_id,NULL);
					}
				}
			}
			token = strtok(NULL," ");									//update token to next input
		}
		if (progress == 0){
				printf("Invalid Command.\n");
		}
				
		else if (progress == -1 || end == -1){
			break;
		}
		else if (progress == 1 && fromfile == 1){
			break;
		}	
		linecount++;						//update line count
	}

	return 0;	

}
