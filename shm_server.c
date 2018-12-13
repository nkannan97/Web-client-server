#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <signal.h>
#include<unistd.h>
// ADD NECESSARY HEADERS
#define SHM_NAME "kannan_murali"
#define PAGESIZE 4096

// Mutex variables
pthread_mutex_t* mutex;
pthread_mutexattr_t mutexAttribute;

// counter for iterations
int counter = 0;

//Process structure
typedef struct {
	int pid;
	char birth[25];
	char clientString[10];
	int elapsed_sec;
	double elapsed_msec;
} stats_t;

// pointer to the client structure
stats_t *pointer;

//Stop the server process, and therefore clear the shared page that it
//created
void exit_handler(int sig) 
{
	//Also use munmap( ) to get rid of the mapping
	munmap(pointer, PAGESIZE);
	//Clear the shared memory region using shm_unlink
	shm_unlink(SHM_NAME);
	exit(0);
}


int main(int argc, char *argv[]) 
{
    
	//Use sigaction() here and call exit_handler
	struct sigaction act;
	act.sa_handler = exit_handler;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
        sigaction(SIGKILL, &act, NULL);
	//sigaction(SIGSEGV, &act, NULL);
	
	// Creating a new shared memory segment
	int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);

	// exit if invalid file descriptor
        if(fd == -1) {
		exit(0);
	}
	//Get the page size
	int pageSize = getpagesize();
	
	if( ftruncate(fd, pageSize) == -1){

		exit(0);
	}
	
	
	//Create the mapping
	pointer = (stats_t*)mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
      
  	// exit if mmap fails 
	if(pointer == MAP_FAILED){

		exit(0);

	}

	mutex = (pthread_mutex_t*) pointer;
	int size = 64;
	// number of segments in the memory page
	int Segments = (PAGESIZE/size);
	
	// setting up  the empty segment
	for(int i = 1; i < Segments; i++){

		pointer[i].pid = -1;

	}
        // Initializing mutex
	pthread_mutexattr_init(&mutexAttribute);
	pthread_mutexattr_setpshared(&mutexAttribute, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(mutex, &mutexAttribute);


	//Enter the infinte loop to start reading the contents of the shared memory
	while (1){
		sleep(1);
		
		// update stats of the process	
		for(int i =1; i < Segments; i++){

			if(pointer[i].pid != -1){
				
				printf("%d, pid : %d, birth : %s, elapsed : %d s %0.4lf ms, %s\n", counter, pointer[i].pid, pointer[i].birth,
						pointer[i].elapsed_sec, pointer[i].elapsed_msec, pointer[i].clientString);
				
			}

		}
		
		counter++;

	}
	
      

    return 0;
}

