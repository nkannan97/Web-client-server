#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include<stdlib.h>
#include<stdio.h>

// ADD NECESSARY HEADERS
#define SHM_NAME "kannan_murali"
//#define MAX_CLIENTS 63

// Mutex variables
pthread_mutex_t* mutex;

//Client structure
typedef struct {
	int pid;
	char birth[25];
	char clientString[10];
	int elapsed_sec;
	double elapsed_msec;
} stats_t;

//Global pointer
stats_t *pointer;

//Global variable that indexes to the client segment
int locateSeg;


void exit_handler(int sig) {
	// ADD

	// Client leaving; needs to reset its segment

	// critical section begins
	pthread_mutex_lock(mutex);
	
	// reseting the segment
	pointer[locateSeg].pid = -1;

	pthread_mutex_unlock(mutex);
	exit(0);
}

int main(int argc, char *argv[]) {

	//Exit on invalid number of arguments
	if(argc != 2){
		exit(0);
	}

	//Use sigaction() here and call exit_handler
	struct sigaction act;
	act.sa_handler = exit_handler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGKILL, &act, NULL);

	struct timeval start,end;
	// opening shared memory page
	int fd = shm_open(SHM_NAME, O_RDWR, 0660);
	// checking if the file descriptor is valid
	if(fd == -1) {
		exit(0);
	}
	// get the page size 
	int pageSize = getpagesize();
	// create a memory map which returns the starting address of the VAS
	pointer = (stats_t*)mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	mutex = (pthread_mutex_t *) pointer;
        // check if the starting address is valid
	if(pointer == MAP_FAILED){

		exit(0);
	}
        
        
	int size = 64;
	// size of each segment
        int Segments = pageSize/size;
	// count the number of active clients 
	int count = 0;
	
	for(int i =1;i<Segments;i++) {

		if(pointer[i].pid !=-1) {
			count++;	
			}

		}
	// if number of clients is the maximum number of clients ( 1st segment for mutex) 
	if(count == Segments -1) {

		exit(0);

		}

	//Critical section starts
	pthread_mutex_lock(mutex);
    
	// search for empty segment
	for(int i = 1; i < Segments; i++){

		if(pointer[i].pid == -1){
			locateSeg = i;

			//Get the pid of the client     

			pointer[i].pid = getpid();
			break;
		}
	        
	}
	pthread_mutex_unlock(mutex);
	//Critical section ends
	
	//Get birth time
	gettimeofday(&start,NULL);
	time_t curtime;
	time(&curtime);	
	//Update the last character in the birth and client name strings to end of string character
	strcpy(pointer[locateSeg].birth, ctime(&curtime)); 
	// break at the new line character
	strtok(pointer[locateSeg].birth,"\n");
        // if the argument is invalid
	if(argv[1] == NULL) {
	    exit(0);
	}	
	// if clientString length is greater than 9, exit
	if(strlen(argv[1]) > 9) {
	    exit(0);
	}
	// copy the string into the ClientString variable
	strcpy(pointer[locateSeg].clientString, argv[1]);
	
	// check if the argument is valid
	//if(argv[1] == NULL) {
	//	exit(0);
	//}
	//Initializing Elapsed second
	pointer[locateSeg].elapsed_sec = 0;
	//Initializing Elapsed msecond
	pointer[locateSeg].elapsed_msec = 0;

	while (1) {
		sleep(1);
		//Update the stats for the particular client
		gettimeofday(&end, NULL);
		pointer[locateSeg].elapsed_sec = end.tv_sec - start.tv_sec;
		pointer[locateSeg].elapsed_msec = (end.tv_usec - start.tv_usec)/1000.0f;

		printf("Active clients : ");
		for(int i = 1; i < Segments; i++){

			//If the segment is valid
			if(pointer[i].pid != -1){

				//Print the PIDs
				printf("%d ", pointer[i].pid);

			}
		}
		printf("\n");
	}


	return 0;
}
