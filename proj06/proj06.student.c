/************************** 
 * Alex Taylor
 * Consumer/producer Program
 * 
 * Correctly ran 17 times in a row. With only clear error being duplicate error message in log file.
 * After creating a copy and renaming to 'proj06', it no longer produces the correct thread numbers to be printed into the log file
 * 
 * Project 06
***************************/
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <semaphore.h>

 //Structs and Global Variables
 
sem_t s;	// locks/unlocks when pushing/popping from buffer
sem_t n;	//guards consumers
sem_t e;	//guards producers
sem_t p;	//guards producer count
sem_t f; 	//stops producers if buffer is full

int p_threads_running;													//global to keep track of producer threads currently running

int starts_with(const char* a,const char* b){
	if (strncmp(a, b, strlen(b)) == 0) {
		return 1;
	}
	return 0;
}

struct account{
	int num;
	double bal;
};

struct p_struct{
	int thread_num;
	char* filename;
};

struct trans{
	int account_num;
	char type[16];
	double dollars;
	int thread_number;
};

struct accounts_database{
	struct account accounts[512];
	int size = 0;
};

struct accounts_database all_accounts; 									//global variable for all accounts

struct cbuff {
	int head = 0;
	int tail = 0;
	int size = 0;
	int capacity = 5;
	struct trans transactions[20];
};

struct cbuff main_buff;													//global variable for circular buffer

void accounts_push (account data){
	all_accounts.accounts[all_accounts.size] = data;
	all_accounts.size++;
}

int buff_full() {
	if (main_buff.size == main_buff.capacity){
		return 1;
	}
	return 0;
}

void buff_print (){
	int val_n = 0;
	sem_getvalue(&n,&val_n);
	int val_p = 0;
	sem_getvalue(&p,&val_p);
	int val_s = 0;
	sem_getvalue(&s,&val_s);
	printf("Sems n:%d  p:%d  s:%d \n", val_n, val_p,val_s);
	printf("head: %d   tail: %d size: %d\n" , main_buff.head, main_buff.tail,main_buff.size);
	for(int i=0; i < main_buff.capacity; i++){
			printf("T ID: %d ", main_buff.transactions[i].thread_number);
			printf(" Acc Num: %d",main_buff.transactions[i].account_num);
			printf("  Type: %s",main_buff.transactions[i].type);
			printf("  Dollars: %.02f ",main_buff.transactions[i].dollars);
			printf("\n");	
	}
}

void buff_push(trans data) {
	if (buff_full()){
		main_buff.head = main_buff.capacity;
	}
	else{
		main_buff.transactions[main_buff.head] = data;
		main_buff.size += 1;
		main_buff.head  = (main_buff.head +1) % main_buff.capacity;
	}
}

void buff_pop (){
	/**
	 * Note trans object is not actually removed from the buffer, 
	 * tail is adjusted so it will be overwritten at some point, 
	 * the data is still in the buffer, but we updated size to
	 * reflect that it is gone.
	 * */
	 main_buff.transactions[main_buff.tail].account_num = 0;
	 main_buff.transactions[main_buff.tail].dollars = 0.0;
	 main_buff.transactions[main_buff.tail].thread_number = 0;
	 strcpy(main_buff.transactions[main_buff.tail].type ,"NULL");
	 	 	 
	
	main_buff.tail = (main_buff.tail +1) % main_buff.capacity;
	main_buff.size -= 1;
}

void account_print(){
	for(int i = 0; i < all_accounts.size; i++){
		printf("%d %.02f \n" , all_accounts.accounts[i].num, all_accounts.accounts[i].bal);
	}
}

void* consumers(void *cons){	
	int errortype = 0;
	FILE* fplog;
	int exists = 0;
	fplog = fopen("accounts.log","w");
	if (fplog== NULL){
		printf("Unable to open accounts.log file \n");
	}
	else{
		fprintf(fplog,"%s  %s  %s  %s  \n" , "Thread" ,"Account", "Transaction", "Amount");
	}
	int i = 0;
	while (main_buff.size > 0 || p_threads_running > 0){			//iterating through file line at a time
		errortype = 0;
		i++;
		if (main_buff.size == 0 && p_threads_running == 0){
			break;
		}
		if (main_buff.size == 0 && p_threads_running > 0){
			sem_wait(&n);
		}
		sem_wait(&n); 
		
		exists = 0;
		int i = main_buff.tail;
		if (strcmp(main_buff.transactions[i].type, "open") == 0){
			for(int j = 0; j < all_accounts.size; j++){
				if (all_accounts.accounts[j].num == main_buff.transactions[i].account_num) {
					exists = 1;
					//account already exists, transaction not kept
					errortype = 1;
				}
			}
			if (exists == 0){
				fprintf(fplog,"%d       %d     %s         %.02f \n" , main_buff.transactions[i].thread_number, main_buff.transactions[i].account_num, main_buff.transactions[i].type, main_buff.transactions[i].dollars);
				struct account new_account;
				new_account.num = main_buff.transactions[i].account_num;
				new_account.bal = main_buff.transactions[i].dollars;
				accounts_push(new_account);
			}
		}
		else if (strcmp(main_buff.transactions[i].type,"withdraw") == 0){
			for(int j = 0; j < all_accounts.size; j++){
				if (all_accounts.accounts[j].num == main_buff.transactions[i].account_num) {
					double result = (all_accounts.accounts[j].bal - main_buff.transactions[i].dollars);
					if (result >= 0){
						fprintf(fplog,"%d       %d     %s     %.02f \n" , main_buff.transactions[i].thread_number, main_buff.transactions[i].account_num, main_buff.transactions[i].type, main_buff.transactions[i].dollars);
						
						all_accounts.accounts[j].bal = result;
					}
					else{
						//Transaction was voided, not enough money in account
						errortype = 2;

					}
				}
			}
		}
		else if (strcmp(main_buff.transactions[i].type, "deposit") == 0){
			for(int j = 0; j < all_accounts.size; j++){
				if (all_accounts.accounts[j].num == main_buff.transactions[i].account_num) {
					double result = (all_accounts.accounts[j].bal + main_buff.transactions[i].dollars);
					all_accounts.accounts[j].bal = result;
				
					fprintf(fplog,"%d       %d     %s      %.02f \n" , main_buff.transactions[i].thread_number, main_buff.transactions[i].account_num, main_buff.transactions[i].type, main_buff.transactions[i].dollars);
				}
				else{
					//Transaction was voided, the account does not exist
					errortype = 3;

				}
			}
		}
		fflush(fplog);
		sem_wait(&s);  
		if (errortype == 1){
			fprintf(fplog,"%d       %d     %s         %.02f  VOIDED\n" , main_buff.transactions[i].thread_number, main_buff.transactions[i].account_num, main_buff.transactions[i].type, main_buff.transactions[i].dollars);
		}
		if (errortype == 2){
			fprintf(fplog,"%d       %d     %s     %.02f  VOIDED\n" , main_buff.transactions[i].thread_number, main_buff.transactions[i].account_num, main_buff.transactions[i].type, main_buff.transactions[i].dollars);
						
		}
		if (errortype ==3){
			fprintf(fplog,"%d       %d     %s      %.02f  VOIDED\n" , main_buff.transactions[i].thread_number, main_buff.transactions[i].account_num, main_buff.transactions[i].type, main_buff.transactions[i].dollars);
		}
		buff_pop();
		sem_post(&s);

	}
	sem_post(&e);
	pthread_exit(NULL);
}

void* producers(void *pros){
	struct p_struct *arguments = (struct p_struct *)pros;
	FILE* fp;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	char* input;
	char* ptr;
	char* filename = (char*)malloc(256);
	int tempint;
	double tempdoub;
	char* ptr2;
	strcpy(filename, get_current_dir_name());	
	strcat(filename,"/");
	strcat(filename,arguments->filename);
	int thread_num = filename[strlen(filename) - 1] - 48;

	fp = fopen(filename,"r");
	free(filename);
	if (fp == NULL){									//unable to open file for whatever reason
		printf("Unable to open proper transN file");
	}
	else{
		
		read = getline(&line, &len, fp);
		while(read != -1){					//iterating through file line at a 
			if (main_buff.size == main_buff.capacity){
				sem_wait(&f);
			}
			input = strtok(line, " ");
			struct trans new_transaction;				//create a transaction struct to hold data
			new_transaction.thread_number = thread_num;
			tempint = strtol(input,&ptr,10);
			new_transaction.account_num = tempint;		//add account number to new_transaction
			input = strtok(NULL," ");
			strcpy(new_transaction.type,input);			//add the transaction type to the struct
			input = strtok(NULL," ");
			tempdoub = strtod(input,&ptr2);				
			new_transaction.dollars = tempdoub;			//add the dollar amount to the struct
			sem_wait(&s);
			buff_push(new_transaction);					//push the entire transaction into the buffer
			sem_post(&n);
			sem_post(&s);
			read = getline(&line, &len, fp);
		}
		fclose(fp);
		if(line != NULL){
			free(line);
		}
	}
	
	sem_wait(&p);
	p_threads_running--;
	sem_post(&p);

	if (main_buff.size > 0){
			int val_n = 0;
			sem_getvalue(&n,&val_n);
			for( int i = main_buff.size; i != val_n; i--){
				sem_post(&n);
			}
		}
	pthread_exit(NULL);
}

int main( int argc, char *argv[] ){		
	//variables involving semaphores, the trans files and arguments
	sem_init(&s,0,1);	
	sem_init(&p,0,1);	
	sem_init(&n,0,0);
	sem_init(&e,0,0);
	sem_init(&f,0,0);
	char* ptr;
	int p_count;
	int b_count;
	DIR *dp;
	char* temp;
	char* file_name[256];
	char* cwd = get_current_dir_name();	
	//variables involving account.old file and its information
	FILE* fpold;
	char* lineold = NULL;
	size_t lenold = 0;
	ssize_t readold;
	char* inputold;
	char* ptrold;
	char* ptr2old;
	char* filenameold = (char*)malloc(256);
	int tempint;
	double tempdoub;
	int acc_num_old;
	double acc_bal_old;
	strcpy(filenameold, get_current_dir_name());	
	strcat(filenameold,"/accounts.old");
	fpold = fopen(filenameold,"r");
	//variables involving account.new file
	FILE* fpnew;
	
	//creating account data_base from old file
if (fpold == NULL){									//unable to open file
		printf("Unable to open accounts.old file \n");
	}
	else{
		readold = getline(&lineold, &lenold, fpold);
		while(readold != -1){								//iterating through file line at a time.
			
			struct account old_account;
			
			inputold = strtok(lineold, " ");
			tempint = strtol(inputold,&ptrold,10);			//converting char* to int and putting the account number into account struct
			acc_num_old = tempint;
			old_account.num = acc_num_old;
			
			inputold = strtok(NULL," ");
			tempdoub = strtod(inputold,&ptr2old);				//converting char* to double and putting the account balance into account struct
			acc_bal_old = tempdoub;
			old_account.bal = acc_bal_old;
			
			accounts_push(old_account);					//adding the account to the total list of accounts
			
			readold = getline(&lineold, &lenold, fpold);
		}
		fclose(fpold);
		if(lineold != NULL){
			free(lineold);
		}
	}
	

	for(int i = 0; i<argc; i++){
		if (strcmp(argv[i], "-p") == 0){
				i++;		  											//update token to next input
				if (argv[i] != NULL){				    				//if there is something after -p...
					char* number = argv[i];
					if (isdigit(*number)){								//check if it is a number.
						p_count = strtol(number,&ptr,10);				//save number as integer 
					}
					else{
						printf("invalid input a number is required after -p. \n");
					}
				}
				else{
					p_count = 1;
				}
		}
		
		if (strcmp(argv[i], "-b") == 0){
				i++;		  											//update token to next input
				if (argv[i] != NULL){				    				//if there is something after -p...
					char* number = argv[i];
					if (isdigit(*number)){								//check if it is a number.
						b_count = strtol(number,&ptr,10);				//save number as integer 
					}
					else{
						printf("invalid input a number is required after -b. \n");
					}
				}
				else{
					b_count = 5;
				}
		}
	}
	
	if (b_count){
		if (b_count > 0 && b_count < 21){
			main_buff.capacity = b_count;
		}
	}

	pthread_t cthread_id = 0;
	pthread_create(&cthread_id, NULL, &consumers,(void *) &p);
																		//this chunk of code gets the current working directory
	dp = opendir(cwd); 													//and iterates over every file in the directory. Creates necessary threads
	if (dp != NULL){														
		for (int i = 0; i < p_count; i++){
				struct dirent *ep;		
				while (ep = readdir(dp)){										
					*file_name = ep->d_name;
					if (starts_with(*file_name,"trans")){						//it then creates int thread_num based on the number in the transN filename							
						struct p_struct p;
						temp = *file_name;
						p.filename = temp;
						p.thread_num = temp[strlen(temp) - 1] - 48;				//storing thread ID to use in producer
						
						if (p.thread_num < 10 && p.thread_num > 0) {
							if (p.thread_num == i){
								p_threads_running++;
								pthread_t pthread_id = i;		

								pthread_create(&pthread_id, NULL, &producers,(void *) &p);
							}
						}
					}	
			}	
			rewinddir(dp);		//sets dp back to starting point
		}
		(void) closedir (dp);		
	}
	else {
		printf("could not open directory \n");	
	}	


	//filling new file with updated accounts
	sem_wait(&e); 
	fpnew = fopen("accounts.new","w");
	if (fpnew == NULL){
		printf("Unable to open accounts.new file \n");
	}
	else{	
		for(int i = 0; i < all_accounts.size; i++){
			fprintf(fpnew,"%d %.2f \n", all_accounts.accounts[i].num, all_accounts.accounts[i].bal);
		}
	}

	free(filenameold);

}

