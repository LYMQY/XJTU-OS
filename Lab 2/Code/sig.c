#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

int flag = 0;
pid_t pid1 = -1, pid2 = -1;

void inter_handler(int sig) {
	flag = 1;
	switch(sig)
	{
		case SIGINT:
			printf("\n2 stop test\n");
			break;
		case SIGQUIT:
			printf("\n3 stop test\n");
			break;
		case SIGSTKFLT:
			printf("\n16 stop test\n");
			break;
		case SIGCHLD:
			printf("\n17 stop test\n");
			break;
		case SIGALRM:
			printf("\n14 stop test\n");
			break;
		default: break;
	}
}

void waiting() {
	while (flag == 0) {
		pause();
	}
}

int main() {
	signal(SIGQUIT, inter_handler);
	signal(SIGINT, inter_handler);
	signal(SIGALRM, inter_handler);

	while (pid1 == -1) pid1 = fork();

	if(pid1 > 0) {
		while (pid2 == -1) pid2 = fork();
		if (pid2 > 0) {
			// Parent Process
			alarm(5);
			waiting();
			//sleep(5);
			
			kill(pid1, 16);
			kill(pid2, 17);
			
			wait(NULL);
			wait(NULL);
			printf("\nParent process is killed!!\n");
		} else {
			// Child Process2
			signal(17, inter_handler);
			signal(SIGQUIT, SIG_IGN);
			signal(SIGINT, SIG_IGN);
			signal(SIGALRM, SIG_IGN);
			pause();
			printf("\nChild process2 is killed by parent!!\n");
			return 0;
		}
	} else {
		// Child Process1
		signal(16, inter_handler);                                                        
		signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGALRM, SIG_IGN); 
		pause(); 
		printf("\nChild process1 is killed by parent!!\n");
		return 0;
	}
	return 0;
}