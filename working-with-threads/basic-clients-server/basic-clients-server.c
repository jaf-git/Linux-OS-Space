#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_THREADS 100
#define CONNECTION_SLEEP_DURATION 3 
#define MAX_REQ_SLEEP_DURATION 2

int exit_prog = 0;

void* check_for_exit(void* arg)
{
    // while(getchar() != 'x');
    while(1);
    // exit_prog = 1;
    // printf("Control thread %lu is finished.\n", pthread_self());
    return NULL;
}

int generate_sleep_duration()
{
    srand(time(NULL));
    return rand() %MAX_REQ_SLEEP_DURATION + 1;
}

void* handle_request(void* arg)
{
    pthread_t thread_id = pthread_self();
    do{
        printf("[Client thread %lu] starting...\n", thread_id);
        sleep(generate_sleep_duration());
        printf("[Client thread %lu] ending...\n", thread_id);
        return NULL;
    } while(!exit_prog);
    printf("Client thread %lu thread is finished.\n", thread_id);
    return NULL;
}

void* recieve_requests(void* arg)
{
    do
    {
        pthread_t thread;
        if(pthread_create(&thread, NULL, handle_request, NULL))
        {
            perror("Error handling clients request\n");
            break;
        }
        sleep(CONNECTION_SLEEP_DURATION);
    }while(!exit_prog);
    printf("The connection %lu thread is finished.\n", pthread_self());
    return NULL;
}

int isNumber(char* str)
{
    while(*str)
    {
        if(!isdigit(*str))
            return 0;
        str++;
    }
    return 1;
}

void* do_nothing(void* arg)
{

    printf("Thread %lu has been created.\n", pthread_self());
    sleep(10);
    printf("Thread %lu is finished.\n", pthread_self());
    return NULL;
}

int create_threads(int num_threads)
{   
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for(int i = 0; i < num_threads; i++)
    {
        if(pthread_create(&threads[i], NULL, do_nothing, NULL))
        {
            free(threads);
            perror("Error creating a thread");
            exit(-1);
        }
    }
    
    int j =0;
    while(threads[j])
    {
        pthread_join(threads[j], NULL);
    }
    free(threads);
    return 1;
}

void* get_threads_number(void* arg)
{
    char* str = (char*)malloc(20 * sizeof(char));
    int threads_num = 0;
    while(!exit_prog)
    {
        scanf("%s", str);
        if(strcmp(str, "x") == 0)
        {
            exit_prog = 1;
            break;
        }

        if(!isNumber(str))
            printf("Please enter a valid number\n");

        threads_num = atoi(str);
        if(threads_num < 0 || threads_num > MAX_THREADS)
        {
            printf("Invalid number of threads, at most 100 threads allowed.\n");
            threads_num = 0;
        }
        
        create_threads(threads_num);
    }
    free(str);
    return NULL;
}

int main() {

    // CREATE CONTROL THREAD THAT WAITS FOR 'X' TO BE PRESSED TO EXIT THE PROGRAM
    pthread_t control_thread;
    if(pthread_create(&control_thread, NULL, check_for_exit, NULL))
    {
        perror("Error creating control thread\n");
        exit(-1);
    }

    // CREATE CONNECTION THREAD THAT WAITS FOR CLIENTS REQ. AND GENERATES PERIODICALLY CLIENT THREADS 
    pthread_t connection_thread;
    if(pthread_create(&connection_thread, NULL, recieve_requests, NULL))    
    {
        perror("Error creating connection thread\n");
        exit(-1);
    }

    // CREATE I/O THREAD TO TAKE N NUMBER OF THREADS
    pthread_t io_thread;
    if(pthread_create(&io_thread, NULL, get_threads_number, NULL))
    {
        perror("Error creating the I/O thread\n");
        exit(-1);
    }

    pthread_join(control_thread, NULL);
    pthread_join(connection_thread, NULL);
    pthread_join(io_thread, NULL);


    printf("The main thread is finished.\n");

    return 0;
}

