#include <stdio.h> 
#include <pthread.h>
#include <unistd.h>

// this method is used in order to avoid overriding the CPU threads
void* go_to_sleep(void* arg)
{
    while(1)
    {
        sleep(1);
    }
}

int main()
{   
    int i;
    for(i = 0;;)
    {
        pthread_t thread;
        int created = pthread_create(&thread, NULL, go_to_sleep, NULL);
        if(created == 0)
            i++;
        else break;
    }
    printf("The maximum number of threads created is : %d\n", i);
    return 0;
}