/*
process2.c 图1-1代码，增加全局变量
在父子进程中对全局变量进行不同操作
*/
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<unistd.h>

int value = 0;

int main()
{
    pid_t pid, pid1;

    /* fork a child process*/
    pid = fork();

    if(pid < 0) {
        /* error occurred */
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) {
        /* child process */
        pid1 = getpid();
        value += 1;
        printf("child: pid = %d ", pid);
        printf("child: value = %d ", value);
        printf("child: *value = %p ", &value);
        printf("child: pid1 = %d\n", pid1);
    }
    else {
        /* parent process */
        pid1 = getpid();
        value += 2;
        printf("parent: pid = %d ", pid);
        printf("parent: value = %d ", value);
        printf("parent: *value = %p ", &value);
        printf("parent: pid = %d\n", pid1);
        wait(NULL);
    }

    return 0;
}