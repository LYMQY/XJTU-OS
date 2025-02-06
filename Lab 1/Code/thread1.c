/* thread1.c 增加了PV操作 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t t; /* create signal */

long int value = 0;

void *func1(void *); /*Thread 1*/
void *func2(void *); /*Thread 2*/

int main()
{
    pthread_t thread1, thread2; /*The thread tid*/
    pthread_attr_t attr1, attr2;

    pthread_mutex_init(&t, NULL);
    pthread_attr_init(&attr1);
    pthread_attr_init(&attr2);

    /* create the thread */
    pthread_create(&thread1, &attr1, func1, NULL);
    pthread_create(&thread2, &attr2, func2, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("variable result: %d\n", value);

    pthread_mutex_destroy(&t);

    return 0;
}

void *func1(void* arg) {
    printf("Thread 1 create success!\n");

    long int i = 0;
    for(i = 0; i < 100000; i++) {
        pthread_mutex_lock(&t);
        value += 100;
        pthread_mutex_unlock(&t);
    }

    pthread_exit(0);
}

void *func2(void* arg) {
    printf("Thread 2 create success!\n");

    long int i;
    for(i = 0; i < 100000; i++) {
        pthread_mutex_lock(&t);
        value -= 100;
        pthread_mutex_unlock(&t);
    }

    pthread_exit(0);
}
