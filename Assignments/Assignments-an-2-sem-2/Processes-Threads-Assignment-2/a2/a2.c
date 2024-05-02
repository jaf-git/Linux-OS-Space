#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "a2_helper.h"

//_____HELPER METHODS SECTION STARTS_____//

void print(char* msg)
{
    printf("%s\n", msg);
}

//_____HELPER METHODS SECTION ENDS_____//


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

int main(int argc, char** argv)
{
    init();
    print("main has started");
    info(BEGIN, 1, 0);

    for(int i = 2; i < 6; i++)
    {
        create_new_process(i);
    }

    info(END, 1, 0);
    return 0;
}