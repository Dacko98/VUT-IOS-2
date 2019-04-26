#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdio.h>


typedef struct param
{
    int personsCount;
    int hackersTime;
    int serfsTime;
    int cruiseTime;
    int returnTime;
    int pierCapacity;
}parameters;

//struktura pre samafory
typedef struct sem{
    sem_t *mutex;
    sem_t *write;
    sem_t *accessPier;
    sem_t *serfQueue;
    sem_t *hackerQueue;
    sem_t *barrier;
}semaphores;

typedef struct shMem{
    int SMserfId;
    int *serfIdcount;

    int SMhackerId;
    int *hackerIdcount;

    int SMserfsPier;
    int *serfsPier;

    int SMhackersPier;
    int *hackersPier;

    int SMoperationNumber;
    int *operationNumber;

    int SMbarrier;
    int *barrier;

    int SMserfBoard;
    int *serfBoard;

    int SMhackerBoard;
    int *hackerBoard;
}sharedMemory;

int procesParam(int argc, char *argv[], parameters *param);

int clearMem(semaphores *semaphores, sharedMemory *memory);

int allocMem(semaphores *semaphores, sharedMemory *memory);

void handleError(int error);

int generatePersons(parameters *param, semaphores *sem, sharedMemory *mem, bool isHacker);

int personLive(parameters *param, semaphores *sem, sharedMemory *mem, bool isHacker);