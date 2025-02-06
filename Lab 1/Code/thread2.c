/* thread2.c 增加system(), exec 族函数 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>

pthread_mutex_t t; /* create signal */

pthread_t thread1, thread2; /*The thread tid*/

void *func1(void *); /*Thread 1*/
void *func2(void *); /*Thread 2*/

int main()
{

    pthread_attr_t attr1, attr2;

    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);
    pthread_mutex_init(&t, NULL);

    /* create the thread */
    pthread_create(&thread1, &attr1, func1, NULL);
    pthread_create(&thread2, &attr2, func2, NULL);

    pthread_join(thread1, NULL);

    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&t);

    return 0;
}

void *func1(void* arg) {
    printf("Thread 1 create success!\n");

    printf("Thread 1 tid = %d, pid = %d\n", pthread_self(), getpid());

    pthread_mutex_lock(&t);
    //execl("./system_call", NULL);
    system("./system_call");
    pthread_mutex_unlock(&t);

    printf("Thread 1 systemcall return\n");

    pthread_exit(0);
}

void *func2(void* arg) {
    printf("Thread 2 create success!\n");

    printf("Thread 2 tid = %d, pid = %d\n", pthread_self(), getpid());

    pthread_mutex_lock(&t);
    //execl("./system_call", NULL);
    system("./system_call");
    pthread_mutex_unlock(&t);

    printf("Thread 2 systemcall return\n");

    pthread_exit(0);
}