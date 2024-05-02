#include <sys/ipc.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/shm.h>
#include "a2_helper.h"

#define P3_THREADS 6
#define P7_THREADS 4
#define P2_THREADS 37
#define P2_MAX_THREADS 5
#define SHARED_MEM_SIZE 10
#define SHARED_P3 6
#define SHARED_P7 4

#define MUTEX 0
#define READ 1 
#define WRITE 2 
#define P2_SEMAPHORE 3

int *shm_buffer;
int semId;
int thread_p7_2_terminated = 0;

pthread_mutex_t p7_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t p7_2_terminated = PTHREAD_COND_INITIALIZER;



void create_trivial_process(int parent_id, int child_id);

//_____HELPER METHODS SECTION STARTS_____//

void print(char* msg)
{
    printf("%s\n", msg);
}

//_____HELPER METHODS SECTION ENDS_____//

void execute_p2(int process_id, int child_id)
{

}

void execute_p3(int process_id, int child_id)
{

}

void create_new_process(int process_id)
{
    int pid;
    if((pid = fork()) < 0)
    {
        perror("Creating a process");
        exit(1);
    }

    if(pid > 0)
    {
        wait(NULL);
    }

    if(pid == 0)
    {
        info(BEGIN, 1, 0);
        printf("The process %d has started..\n", process_id);
        info(END, 1, 0);
    }
}

void execute_main(int parent_id, int child_id, int num_processes)
{
    info(BEGIN, child_id, 0);
    for(int i = 1; i <= num_processes; i++)
    {
        int id = i + 1;
        if(id == 2)
            execute_p2(child_id, id);
        else if(id == 3)
            execute_p3(child_id, id);
        else create_trivial_process(child_id, id);
    }

    int status;
    for(int i = 1; i <= num_processes; i++)
    {
        pid_t pid = wait(&status);
        if(WIFEXITED(status))
        {
            printf("Child process with PID %d terminated\n", pid);
        }
    }
    info(END, child_id, 0);
}

int create_shared_memory(int locations_num)
{
    int shmId;
    if((shmId = shmget(IPC_PRIVATE, locations_num * sizeof(int), IPC_CREAT | 0600)) < 0)
    {
        perror("Shared memory");
        exit(1);
    }
    return shmId;
}

void initialize_shared_memory(int value)
{
    for(int i = 0; i < SHARED_MEM_SIZE; i++)
        shm_buffer[i] = value;
}

void create_trivial_process(int parent_id, int child_id)
{
    pid_t pid;
    if((pid = fork()) < 0)
    {
        perror("Forking");
        exit(3);
    }
    if(pid == 0)
    {
        info(BEGIN, child_id, 0);
        // printf("Parent %d started Child %d\n", parent_id, child_id);
        info(END, child_id, 0);
        exit(EXIT_SUCCESS);
    }   
}

int main(int argc, char** argv)
{
    init();
    // info(BEGIN, 1, 0);
    /*
    Shared Memory Segment   = { 0,    1,    2,    3,    4,    5,    6,    7,    8,    9    };
    Each store a statue for = { T3.1, T3.2, T3.3, T3.4, T3.5, T3.6, T7.1, T7.2, T7.3, T7.4 };
    Thread Statues:
    If Shared location = 0 => Thread did not start yet.
    If Shared location = 1 => Thread has started.
    If Shared location = 2 => Thread Finished its execution.
    */

    int shmId = create_shared_memory(SHARED_MEM_SIZE);

    // assign a pointer to the shared segmnet
    shm_buffer = (int*) shmat(shmId, 0, 0);

    // initialize segment locations with 0, to indicate that no thread has started yet
    initialize_shared_memory(0);

    // create a set of semaphores
    if((semId = semget(IPC_PRIVATE, 4, IPC_CREAT | 0600)) < 0)
    {
        perror("Creating a set of semaphores");
        exit(2);
    }

    // initialize the semaphores
    semctl(semId, MUTEX, SETVAL, 1);
    semctl(semId, READ, SETVAL, 1);
    semctl(semId, WRITE, SETVAL, 1);
    semctl(semId, P2_SEMAPHORE, SETVAL, 5);

    // the main starts here 
    // execute_main(parent_process_id, main_process_id, number_of_processes_to_generate)
    execute_main(0, 1, 4);

    print("Shared mem segment values:");
    for(int i = 0; i < SHARED_MEM_SIZE; i++)
        printf("%d -> %d\n", i, shm_buffer[i]);
    return 0;
}