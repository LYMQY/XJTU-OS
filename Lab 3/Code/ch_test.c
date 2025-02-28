#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include <unistd.h>
#include <signal.h>


int main()
{
    pid_t pid;
    pid=fork();
    if(pid!=0)//父进程，负责写，给别人发送消息
    {
        //printf("this pid is %d\n",getpid());
        int fd,i;
        char msg[100];
        fd= open("/dev/chardev0",O_WRONLY,S_IRUSR|S_IWUSR);
        
        if(fd!=-1)
        {
            while(1)
            {
                printf("I am %d and I want to have a dialogue with the Three Body World!\n",getpid());
                for(i=0;i<101;i++)  //初始化
                    msg[i]='\0';
                printf("Please input the globar:\n");
                fgets(msg, sizeof(msg), stdin);
                msg[strcspn(msg, "\n")] = '\0';
                write(fd,msg,strlen(msg));
                
                if(strcmp(msg,"quit()")==0)
                {
                    close(fd);
                    kill(pid, SIGKILL);
                    break;
                }
            }
        }
        else
        {
            printf("device open failure\n");
        }
        wait(NULL);
        return 0;
    }
    else//子进程，负责读
    {
        int fd,i;
        char msg[101];
        fd= open("/dev/chardev0",O_RDONLY,S_IRUSR|S_IWUSR);
    
        if(fd!=-1)
        {
            while(1)
            {
                
                for(i=0;i<101;i++)  //初始化
                    msg[i]='\0';
            
                read(fd,msg,100);
                printf("id is %d> ",getpid());
                printf("%s\n",msg);
            
                if(strcmp(msg,"quit()")==0)
                {
                    close(fd);
                    break;
                }
            }
        }
        else
        {
            printf("device open failure,%d\n",fd);
        }
        return 0;
    }
}