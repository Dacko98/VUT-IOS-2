#include "proj2.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

FILE *output = NULL;

int main(int argc, char *argv[])
{
    parameters param;
    semaphores sem;
    sharedMemory mem;

    int errCode =0;
    output = fopen("proj2.out", "w+");
    if (output == NULL) {
        fprintf(stderr, "number 3\n");
        //handleError(-6, output);
        return -1;
    }


   errCode= procesParam(argc, argv, &param);
    if (errCode !=0){
        fprintf(stderr, "number 4\n");
        //handleError(errCode, output);
        return -1;
    }

   errCode= allocMem(&sem, &mem);
    if (errCode !=0){
        fprintf(stderr, "number 5\n");
        //handleError(errCode, output);
        return -1;
    }

    *mem.operationNumber = 0;
    *mem.serfIdcount= 0;
    *mem.hackerIdcount= 0;
    *mem.hackersPier= 0;
    *mem.serfsPier= 0;
    *mem.serfBoard= 0;
    *mem.hackerBoard= 0;
    *mem.barrier=4;

    pid_t hackerPid = fork();

    if(hackerPid<0){
        fprintf(stderr, "hackerPid<0");
       ////errCode= -5;
        kill(0, SIGKILL);
    } else if(hackerPid ==0){
            sem_wait(sem.semWrite);
            fprintf(output, "hack\n");
            sem_post(sem.semWrite);
            generatePersons(&param, &sem, &mem, true);
    } else {           
        pid_t serfPid = fork();
        sem_wait(sem.semWrite);
        fprintf(output, "serf\n");
            sem_post(sem.semWrite);
        if(serfPid<0){
            sem_wait(sem.semWrite);
            fprintf(output, "sigSerf\n");
            sem_post(sem.semWrite);
            fprintf(stderr, "serfPid<0");
            //errCode = -5;
            kill(0, SIGKILL);
        } else if(serfPid ==0){
            sem_wait(sem.semWrite);
            fprintf(output, "generatserf\n");
            sem_post(sem.semWrite);
            generatePersons(&param, &sem, &mem, false);

        }
        else{
            //fprintf(output, "konecne\n");

        }
        
        
    }
    //wait(NULL);

    if(errCode!=0){
        fprintf(stderr, "number 6\n");
        //handleError(errCode, output);
        return -1;
    }

   errCode= clearMem(&sem, &mem);
    if (errCode !=0){
        fprintf(stderr, "number 7 %d\n", errCode);
        //handleError(errCode, output);
        return -1;
    }



    //printf("%d", param.returnTime);
    return 0;
}


int allocMem(semaphores *semaphores, sharedMemory *memory){
  /*  printf("1\n");
    if((semaphores->semWrite= sem_open("/semWrite", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) return -3; 
    printf("2\n");
    if((semaphores->mutex = sem_open("/semMutex", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) return -3; 
    printf("3\n");    
    if((semaphores->hackerQueue = sem_open("/semQueue", O_CREAT | O_EXCL, 0644, 0)) == SEM_FAILED) return -3; 
    if((semaphores->serfQueue= sem_open("/serfQueue", O_CREAT | O_EXCL, 0644, 0)) == SEM_FAILED) return -3; 
    if((semaphores->accessPier= sem_open("/accessPier", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) return -3; 
    if((semaphores->barrier= sem_open("/barrier", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) return -3; 
*/
    if ((semaphores->accessPier = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) return -3;
    if ((semaphores->barrier= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) return -3;
    if ((semaphores->hackerQueue= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) return -3;
    if ((semaphores->mutex= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) return -3;
    if ((semaphores->serfQueue= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) return -3;
    if ((semaphores->semWrite= mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) return -3;

    if (sem_init(semaphores->accessPier, 1, 1) == -1) return -3;
    if (sem_init(semaphores->barrier, 1, 1) == -1) return -3;
    if (sem_init(semaphores->hackerQueue, 1, 1) == -1) return -3;
    if (sem_init(semaphores->mutex, 1, 1) == -1) return -3;
    if (sem_init(semaphores->serfQueue, 1, 0) == -1) return -3;
    if (sem_init(semaphores->semWrite, 1, 1) == -1) return -3;

    if((memory->SMserfId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) return-4;
    if((memory->serfIdcount = shmat(memory->SMserfId, NULL, 0)) == NULL) return-4;

    if((memory->SMserfsPier = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) return-4;
    if((memory->serfsPier= shmat(memory->SMserfsPier, NULL, 0)) == NULL) return-4;
    
    if((memory->SMhackersPier = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) return-4;
    if((memory->hackersPier= shmat(memory->SMhackersPier, NULL, 0)) == NULL) return-4;

    if((memory->SMoperationNumber = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) return-4;
    if((memory->operationNumber= shmat(memory->SMoperationNumber, NULL, 0)) == NULL) return-4;

    if((memory->SMbarrier= shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) return-4;
    if((memory->barrier= shmat(memory->SMbarrier, NULL, 0)) == NULL) return-4;

    if((memory->SMserfBoard= shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) return-4;
    if((memory->serfBoard= shmat(memory->SMserfBoard, NULL, 0)) == NULL) return-4;

    if((memory->SMhackerId= shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) return-4;
    if((memory->hackerIdcount= shmat(memory->SMhackerId, NULL, 0)) == NULL) return-4;

    if((memory->SMhackerBoard= shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1) return-4;
    if((memory->hackerBoard= shmat(memory->SMhackerBoard, NULL, 0)) == NULL) return-4;
    

    return 0;
}


int clearMem(semaphores *semaphores, sharedMemory *memory){
    fclose(output);
    fprintf(stderr, "clear");
    sem_unlink("/write");
    sem_unlink("/mutex");
    sem_unlink("/hackerQueue");
    sem_unlink("/serfQueue");
    sem_unlink("/accesPier");
    sem_unlink("/barrier");

    sem_close(semaphores->hackerQueue);
    sem_close(semaphores->serfQueue);
    sem_close(semaphores->accessPier);
    sem_close(semaphores->mutex);
    sem_close(semaphores->semWrite);
    sem_close(semaphores->barrier);

    if(shmctl(memory->SMserfId, IPC_RMID, NULL) == -1) return -5; 
    if(shmctl(memory->SMhackerId, IPC_RMID, NULL) == -1) return -5; 
    if(shmctl(memory->SMserfsPier, IPC_RMID, NULL) == -1) return -5; 
    if(shmctl(memory->SMhackersPier, IPC_RMID, NULL) == -1) return -5; 
    if(shmctl(memory->SMoperationNumber, IPC_RMID, NULL) == -1) return -5; 
    if(shmctl(memory->SMbarrier, IPC_RMID, NULL) == -1) return -5; 
    if(shmctl(memory->SMhackerBoard, IPC_RMID, NULL) == -1) return -5; 
    if(shmctl(memory->SMserfBoard, IPC_RMID, NULL) == -1) return -5; 
    return 0;

}

int procesParam(int argc, char *argv[], parameters *param){
    char *stringValue;
    if (argc != 7){
        return -1;
    } else {
        param->personsCount = strtol(argv[1], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        param->hackersTime = strtol(argv[2], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        param->serfsTime = strtol(argv[3], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        param->cruiseTime = strtol(argv[4], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        param->returnTime = strtol(argv[5], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        param->pierCapacity = strtol(argv[6], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
    }

    return 0;
}

void handleError(int errCode){

    fclose(output);
    switch (errCode)
    {
        case -1:
            fprintf(stderr, "Wrong number of parameters\n");
            break;
        case -2:
            fprintf(stderr, "Type only numbers\n");
            break;
        case -3:
            fprintf(stderr, "Problem with opening semaphores\n");
            break;
        case -4:
            fprintf(stderr, "Problem with memory allocation\n");
            break;
        case -5:
            fprintf(stderr, "Problem with cleaning of the memory\n");
            break;
        case -6:
            fprintf(stderr, "Problem with opening a file\n");
            break;
        case -7:
            fprintf(stderr, "Sigkill\n");
            break;
        default:
            fprintf(stderr, "chyba ");
            break;
    }

}

int generatePersons(parameters *param, semaphores *sem, sharedMemory *mem, bool isHacker){
    //fprintf(stderr, "generatePerson\n");
    int errCode = 0;
    pid_t personPid = 0;
    //fprintf(stderr, "%d\n",param->personsCount);
    for(int i = 0; i < param->personsCount; i++)
    {   
        
        if(param->serfsTime != 0){    
            usleep(rand()%param->serfsTime*1000);
        }
            personPid = fork();
            //fprintf(output, "%d\n", isHacker);


        
        if(personPid<0){
            fprintf(stderr, "number 0\n");
           //errCode= -5;
            kill(0, SIGKILL);
            return -7;
        } else if(personPid ==0){
            //fprintf(output, "child");
           
            //personLive(param, sem, mem, isHacker);
            if(isHacker){
                personWalkH(param, sem, mem);
            } else {
                personWalkS(param, sem, mem);
            }
            

            } /*else {
            fprintf(output, "parent\n");

                       

            }*/
 
    } 
    //wait(NULL);


    if(errCode!=0){
        fprintf(stderr, "number 1 %d \n",errCode);
        //handleError(errCode, output);
        return -1;
    }

    //errCode= clearMem(sem, mem);
    if (errCode !=0){
        fprintf(stderr, "number 2\n");
        //handleError(errCode, output);
        return -1;
    }
    exit(0);
}

int personLive(parameters *param, semaphores *sem, sharedMemory *mem, bool isHacker){
   /* 
    fprintf(output, "t");
    int *idCount;
    int *actualPersonPier;
    int *otherPersonPier;
    int *actualPersonBoard;
    int *otherPersonBoard;
    sem_t *actualPersonQueue;
    sem_t *otherPersonQueue;
    char *txt = "0";

    if(isHacker){
        idCount = mem->hackerIdcount;
        actualPersonPier = mem->hackersPier;
        otherPersonPier = mem->serfsPier;
        actualPersonBoard = mem->hackerBoard;
        otherPersonBoard = mem->serfBoard;
        actualPersonQueue = sem->hackerQueue;
        otherPersonQueue = sem->serfQueue ;

    } else {
        idCount = mem->serfIdcount;
        actualPersonPier = mem->serfsPier;
        otherPersonPier = mem->hackersPier;
        actualPersonBoard = mem->serfBoard;
        otherPersonBoard = mem->hackerBoard;
        actualPersonQueue = sem->serfQueue;
        otherPersonQueue = sem->hackerQueue;
    }


    sem_wait(sem->semWrite);
    //printf("%s", txt);
    int ID = (*idCount)++;
    sem_post(sem->semWrite);

        sem_wait(sem->semWrite);
        fprintf(output, "%d:\t %d:\t start:\t :\t %d\n",++(*mem->operationNumber), *mem->hackersPier, *mem->serfsPier);
        sem_post(sem->semWrite);

    bool isCaptain = false;

    while(1){
        sem_wait(sem->accessPier);
        if(((*actualPersonPier)+(*otherPersonPier))<param->pierCapacity){
            (*actualPersonPier)++;

            sem_post(sem->accessPier);

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t %s %d:\t waits:\t %d:\t %d\n",++(*mem->operationNumber), *txt, ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);
          

            sem_wait(sem->mutex);
            if(++(*actualPersonBoard)==4){
                *actualPersonPier =0;
                *actualPersonBoard=0;
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t %s %d:\t boards:\t %d:\t %d\n",++(*mem->operationNumber), *txt, ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);

                usleep(rand()%param->cruiseTime*1000);

                sem_post(actualPersonQueue);
                sem_post(actualPersonQueue);
                sem_post(actualPersonQueue);
                sem_post(actualPersonQueue);
                isCaptain = true;
            } else if(*actualPersonBoard==2 && *otherPersonBoard >=2){
                *actualPersonPier =0;
                *otherPersonPier-=2;
                usleep(rand()%param->cruiseTime*1000);

                sem_post(actualPersonQueue);
                sem_post(actualPersonQueue);
                sem_post(otherPersonQueue);
                sem_post(otherPersonQueue);

                isCaptain = true;
            } else {
                
                sem_post(sem->mutex);
            }

            sem_wait(actualPersonQueue);


            if (!isCaptain){
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t %s %d:\t member exits:\t %d:\t %d\n",++(*mem->operationNumber), *txt, ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);
            }


            sem_wait(sem->semWrite);
            *(mem->barrier)--;
  
            if(*(mem->barrier) == 0){
               
                sem_post(sem->barrier);
                sem_post(sem->barrier);
                sem_post(sem->barrier);
                sem_post(sem->barrier);
            }
            sem_post(sem->semWrite);
            sem_wait(sem->barrier);
            
             if (isCaptain){
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t %s %d:\t captain exits:\t %d:\t %d\n",++(*mem->operationNumber), *txt, ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                (*mem->barrier)=4;
                sem_post(sem->semWrite);
                sem_post(sem->mutex);
                
            }

            break;

        } else {
            sem_post(sem->accessPier);
            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t %s %d:\t leaves queue:\t %d:\t %d\n",++(*mem->operationNumber), *txt, ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);

            usleep(rand()%param->returnTime*1000);

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t %s %d:\t is back:\t %d:\t %d\n",++(*mem->operationNumber), *txt, ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);

        }
    }*/
    exit(0);
}


int personWalkS(parameters *param, semaphores *sem, sharedMemory *mem){

    sem_wait(sem->semWrite);
    int ID = ++(*mem->serfIdcount);
    sem_post(sem->semWrite);


    sem_wait(sem->semWrite);
    fprintf(output, "%d:\t SERF %d:\t start:\t %d:\t %d\n",++(*mem->operationNumber),ID, *mem->hackersPier, *mem->serfsPier);
    sem_post(sem->semWrite);

    bool isCaptain = false;

    while(1){
        sem_wait(sem->accessPier);
        if(((*(mem->serfsPier))+(*(mem->hackersPier)))<param->pierCapacity){
            (*(mem->serfsPier))++;

            sem_post(sem->accessPier);

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t SERF %d:\t waits:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);
          

            sem_wait(sem->mutex);
            if(++(*(mem->serfBoard))==4){
                *(mem->serfsPier) =0;
                *(mem->serfBoard)=0;
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t SERF %d:\t boards:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);

                usleep(rand()%param->cruiseTime*1000);

                sem_post(sem->serfQueue);
                sem_post(sem->serfQueue);
                sem_post(sem->serfQueue);
                sem_post(sem->serfQueue);
                isCaptain = true;
            } else if(*(mem->serfBoard)==2 && *(mem->hackerBoard) >=2){
                *(mem->serfsPier) =0;
                *(mem->serfBoard)=0;
                *(mem->hackerBoard)=0;
                *(mem->hackersPier)-=2;
                usleep(rand()%param->cruiseTime*1000);

                sem_post(sem->serfQueue);
                sem_post(sem->serfQueue);
                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);

                isCaptain = true;
            } else {
                
                sem_post(sem->mutex);
            }

            sem_wait(sem->serfQueue);


            if (!isCaptain){
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t SERF %d:\t member exits:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);
            }


            sem_wait(sem->semWrite);
            *(mem->barrier)--;
  
            if(*(mem->barrier) == 0){
               
                sem_post(sem->barrier);
                sem_post(sem->barrier);
                sem_post(sem->barrier);
                sem_post(sem->barrier);
            }
            sem_post(sem->semWrite);
            sem_wait(sem->barrier);
            
             if (isCaptain){
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t SERF %d:\t captain exits:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                (*mem->barrier)=4;
                sem_post(sem->semWrite);
                sem_post(sem->mutex);
                
            }

            break;
            exit(0);

        } else {
            sem_post(sem->accessPier);
            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t SERF %d:\t leaves queue:\t %d:\t %d\n",++(*mem->operationNumber),  ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);

            usleep(rand()%param->returnTime*1000);

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t SERF %d:\t is back:\t %d:\t %d\n",++(*mem->operationNumber),  ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);

        }
    }
}

int personWalkH(parameters *param, semaphores *sem, sharedMemory *mem){

    sem_wait(sem->semWrite);
    int ID = ++(*mem->hackerIdcount);
    sem_post(sem->semWrite);


    sem_wait(sem->semWrite);
    fprintf(output, "%d:\t HACK %d:\t start:\t %d:\t %d\n",++(*mem->operationNumber),ID, *mem->hackersPier, *mem->serfsPier);
    sem_post(sem->semWrite);

    bool isCaptain = false;

    while(1){
        sem_wait(sem->accessPier);
        if(((*(mem->serfsPier))+(*(mem->hackersPier)))<param->pierCapacity){
            (*(mem->hackersPier))++;

            sem_post(sem->accessPier);

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t HACK %d:\t waits:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);
          

            sem_wait(sem->mutex);
            if(++(*(mem->hackerBoard))==4){
                *(mem->hackersPier) =0;
                *(mem->hackerBoard)=0;
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t HACK %d:\t boards:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);

                usleep(rand()%param->cruiseTime*1000);

                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);
                isCaptain = true;
            } else if(*(mem->hackerBoard)==2 && *(mem->serfBoard) >=2){
                *(mem->hackersPier) =0;
                *(mem->hackerBoard)=0;
                *(mem->hackerBoard)=0;
                *(mem->serfsPier)-=2;
                usleep(rand()%param->cruiseTime*1000);

                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);

                isCaptain = true;
            } else {
                
                sem_post(sem->mutex);
            }

            sem_wait(sem->hackerQueue);


            if (!isCaptain){
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t HACK %d:\t member exits:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);
            }


            sem_wait(sem->semWrite);
            *(mem->barrier)--;
  
            if(*(mem->barrier) == 0){
               
                sem_post(sem->barrier);
                sem_post(sem->barrier);
                sem_post(sem->barrier);
                sem_post(sem->barrier);
            }
            sem_post(sem->semWrite);
            sem_wait(sem->barrier);
            
             if (isCaptain){
                sem_wait(sem->semWrite);
                fprintf(output, "%d:\t HACK %d:\t captain exits:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                (*mem->barrier)=4;
                sem_post(sem->semWrite);
                sem_post(sem->mutex);
                
            }

            break;
            exit(0);

        } else {
            sem_post(sem->accessPier);
            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t HACK %d:\t leaves queue:\t %d:\t %d\n",++(*mem->operationNumber),  ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);

            usleep(rand()%param->returnTime*1000);

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t HACK %d:\t is back:\t %d:\t %d\n",++(*mem->operationNumber),  ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);

        }
    }
}
