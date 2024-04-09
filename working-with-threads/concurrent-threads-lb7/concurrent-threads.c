#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_THREADS 100

bool exit_prog = false;
int threads_num;

void* display(void* arg)
{
    pthread_t thread_id = pthread_self();
    while(!exit_prog)
    {
        printf("Thread ID is: %lu with process id: %u\n", thread_id, getpid());
        sleep(1);
    }
    printf("Thread %lu is finished.\n", thread_id);
    return NULL;
}

void* detect_input(void* arg)
{   
    while(getchar() != 'q');
    exit_prog = true;
    return NULL;
}

bool create_input_detection_thread(pthread_t* thread)
{   
    int created = pthread_create(thread, NULL, detect_input, NULL);
    if(created != 0)
    {
        perror("Error creating the thread");
        return false;
    }
    return true;
}

int get_threads_num(int argc, char* argv[])
{
    if(argc == 2)
    {
        threads_num = atoi(argv[1]);
    } 
    else 
    {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return 0;
    }

    if(threads_num < 0 || threads_num > MAX_THREADS)
    {
        printf("Invalid number of threads at most 100.\n");
        return 0;
    }
    return 1;
}

int main(int argc, char* argv[])
{   

    if(!get_threads_num(argc, argv))
        return -1;

    pthread_t input_detect;
    if(!create_input_detection_thread(&input_detect))
        return -1;


    pthread_t threads[MAX_THREADS] = {0};
    for(int i = 0; i < threads_num; i++)
    {   
        if(pthread_create(&threads[i], NULL, display, NULL))
        {
            perror("Error occured while creating a thread");
            break;
        }
    }

    for(int j = 0; threads[j] != 0; j++)
        pthread_join(threads[j], NULL);

    pthread_join(input_detect, NULL);
    printf("Thread main is finished.\n");

    return 0;
}