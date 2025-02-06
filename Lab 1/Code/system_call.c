/* system_call.c */
#include<sys/types.h>
#include<stdio.h>
#include<unistd.h>

int main()
{
    pid_t pid;

    /* system_call process */
    pid = getpid();
    printf("system_call: PID = %d\n", pid);

}