/* process4.c 增加exe族函数 system() */
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

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
        //execl("./system_call", NULL);
        //system("./system_call");
        pid1 = getpid();
        printf("child: pid = %d ", pid);
        printf("child: pid1 = %d\n", pid1);
        //execl("./system_call", NULL);
        system("./system_call");
    }
    else {
        /* parent process */
        pid1 = getpid();
        printf("parent: pid = %d ", pid);
        printf("parent: pid = %d\n", pid1);
        wait(NULL);
    }

    return 0;
}