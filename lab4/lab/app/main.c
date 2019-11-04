#include "lib.h"
#include "types.h"

int data = 0;

/*int uEntry(void) {
    
    int i = 4;
    int ret = 0;
    sem_t sem;
    printf("Father Process: Semaphore Initializing.\n");
    ret = sem_init(&sem, 2);
    if (ret == -1) {
        printf("Father Process: Semaphore Initializing Failed.\n");
        exit();
    }

    ret = fork();

    if (ret == 0) {
        while( i != 0) {
            i --;
            printf("Child Process: Semaphore Waiting.\n");
            sem_wait(&sem);
            printf("Child Process: In Critical Area.\n");
        }
        printf("Child Process: Semaphore Destroying.\n");
        sem_destroy(&sem);
        exit();
    }
    else if (ret != -1) {
        while( i != 0) {
            i --;
            printf("Father Process: Sleeping.\n");
            sleep(128);
            printf("Father Process: Semaphore Posting.\n");
            sem_post(&sem);
        }
        printf("Father Process: Semaphore Destroying.\n");
        sem_destroy(&sem);
        exit();
    }
    
    return 0;
}*/

int uEntry(void) {
    
    sem_t empty;
    sem_t full;
    sem_t mutex;

    int ret1,ret2,ret3;
    
    ret1 = sem_init(&empty, 100);
    ret2 = sem_init(&full, 0);
    ret3 = sem_init(&mutex, 1);


    if(ret1 == -1 || ret2 == -1 || ret3 == -1) {
        printf("Father Process: Semaphore Initializing Failed.\n");
        exit();
    }
    
    for(int i = 0; i < 6; i++) {
        int ret = fork();
        if(ret == 0) break;
    }

    int pid = getpid();

    if(pid >= 2 && pid <= 3) {
        for(int i = 1; i <= 8; i++) {
            printf("pid %d, producer %d, produce, product %d\n",pid,pid-1,i);
            sem_wait(&empty);
            printf("pid %d, producer %d, try lock\n",pid,pid-1);
            sem_wait(&mutex);
            printf("pid %d, producer %d, locked\n",pid,pid-1);
            sem_post(&mutex);
            printf("pid %d, producer %d, unlock\n",pid,pid-1);
            sem_post(&full);
            sleep(128);
        }
    }
    else if(pid >= 4 && pid <= 7){
        for(int i = 1; i <= 4; i++) {
            printf("pid %d, consumer %d, try consume, product %d\n",pid,pid-3,i);
            sem_wait(&full);
            printf("pid %d, consumer %d, try lock\n",pid,pid-3);
            sem_wait(&mutex);
            printf("pid %d, consumer %d, locked\n",pid,pid-3);
            sem_post(&mutex);
            printf("pid %d, consumer %d, unlock\n",pid,pid-3);
            sem_post(&empty);
            printf("pid %d, consumer %d, consumed, product %d\n",pid,pid-3,i);
            sleep(128);
        }
    }

    //sem_destroy(&empty);
    //sem_destroy(&full);
    //sem_destroy(&mutex);
    exit();
    return 0;
}