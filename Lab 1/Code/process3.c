/* process.c 增加在return前对全局变量的操作*/
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

    value += 3;
    printf("before return: value = %d ", value);
    printf("before return: *value = %p\n", &value);

    return 0;
}