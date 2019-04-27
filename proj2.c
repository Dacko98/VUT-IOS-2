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
    errCode= allocMem(&sem, &mem);
    if (errCode !=0){
        handleError(errCode, &sem, &mem);
        return -1;
    }

    output = fopen("proj2.out", "w+");
    if (output == NULL) {
        handleError(-6, &sem, &mem);
        return -1;
    }

    errCode= procesParam(argc, argv, &param);
    if (errCode !=0){
        handleError(errCode, &sem, &mem);
        return 1;
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
        handleError(-10, &sem, &mem);
        kill(0, SIGKILL);
        return -1;
    } else if(hackerPid ==0){
            errCode=generatePersonsH(&param, &sem, &mem);
             if (errCode !=0){
                handleError(errCode, &sem, &mem);
                return -1;
            }
        } else {           
            pid_t serfPid = fork();
            if(serfPid<0){
                handleError(-10, &sem, &mem);
                kill(0, SIGKILL);
                return -1;
            } else if(serfPid ==0){
                errCode=generatePersonsS(&param, &sem, &mem);
                 if (errCode !=0){
                    handleError(errCode, &sem, &mem);
                    return -1;
                }
            }
        }

    wait(NULL);

    errCode= clearMem(&sem, &mem);
    if (errCode !=0){
        handleError(errCode, &sem, &mem);
        return -1;
    }

    return 0;
}


int allocMem(semaphores *semaphores, sharedMemory *memory){
  /* 
    if((semaphores->semWrite= sem_open("/semWrite", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) return -3; 
    if((semaphores->mutex = sem_open("/semMutex", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) return -3; 
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
    if (sem_init(semaphores->barrier, 1, 0) == -1) return -3;
    if (sem_init(semaphores->hackerQueue, 1, 0) == -1) return -3;
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
        if(param->personsCount<2 || (param->personsCount%2==1)) return -8;
        param->hackersTime = strtol(argv[2], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        if(param->hackersTime<0 || param->hackersTime>2000) return -8;
        param->serfsTime = strtol(argv[3], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        if(param->serfsTime<0 || param->serfsTime>2000) return -8;
        param->cruiseTime = strtol(argv[4], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        if(param->cruiseTime<0 || param->cruiseTime>2000) return -8;
        param->returnTime = strtol(argv[5], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        if(param->returnTime<20 || param->returnTime>2000) return -8;
        param->pierCapacity = strtol(argv[6], &stringValue, 10);
        if (stringValue[0] != '\0') return -2;
        if(param->pierCapacity<5) return -8;
    }

    return 0;
}

void handleError(int errCode, semaphores *sem, sharedMemory *mem){

    bool memory = true;
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
            memory = false;
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
    if(memory){
        errCode=clearMem(sem, mem);
        if(errCode != 0 )  fprintf(stderr, "Problem with cleaning of the memory\n");
    }

}

int generatePersonsS(parameters *param, semaphores *sem, sharedMemory *mem){
    pid_t personPid = 0;

    for(int i = 0; i < param->personsCount; i++)
    {    
        if(param->serfsTime != 0){    
            usleep(rand()%param->serfsTime*1000);
        }
        personPid = fork(); 
        if(personPid<0){
            kill(0, SIGKILL);
            return -10;
        } else if(personPid ==0){
            personWalkS(param, sem, mem);   
            }
    } 
    wait(NULL);
    exit(0);
}


int generatePersonsH(parameters *param, semaphores *sem, sharedMemory *mem){
    pid_t personPid = 0;

    for(int i = 0; i < param->personsCount; i++)
    {    
        if(param->hackersTime != 0){    
            usleep(rand()%param->hackersTime*1000);
        }
        personPid = fork(); 
        if(personPid<0){
            kill(0, SIGKILL);
            return -10;
        } else if(personPid ==0){
            personWalkH(param, sem, mem);   
            }
    }
    wait(NULL); 
    exit(0);
}


int personWalkS(parameters *param, semaphores *sem, sharedMemory *mem){

    sem_wait(sem->semWrite);
    int ID = ++(*mem->serfIdcount);
    sem_post(sem->semWrite);

    sem_wait(sem->semWrite);
    fprintf(output, "%d:\t SERF %d:\t starts\t\n",++(*mem->operationNumber),ID);
     fflush(output); 
    sem_post(sem->semWrite);

    bool isCaptain = false;

    while(1){
        sem_wait(sem->accessPier);
        if(((*(mem->serfsPier))+(*(mem->hackersPier)))<param->pierCapacity){
         //   (*(mem->serfsPier))++;

          

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t SERF %d:\t waits:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, ++(*mem->serfsPier));
            fflush(output); 
            sem_post(sem->semWrite);
            sem_post(sem->accessPier);
          

            sem_wait(sem->mutex);
            if(++(*(mem->serfBoard))==4){
                
                sem_wait(sem->semWrite);
                *(mem->serfsPier) -=4;
                *(mem->serfBoard)=0;
                fprintf(output, "%d:\t SERF %d:\t boards:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);

                usleep(rand()%param->cruiseTime*1000);

                sem_post(sem->serfQueue);
                sem_post(sem->serfQueue);
                sem_post(sem->serfQueue);
                sem_post(sem->serfQueue);
                isCaptain = true;
            } else if((*(mem->serfBoard)==2 )&&( *(mem->hackerBoard) >=2)){
                sem_wait(sem->semWrite);
                *(mem->serfsPier) -=2;
                *(mem->serfBoard)=0;
                *(mem->hackerBoard)-=2;
                *(mem->hackersPier)-=2;
                fprintf(output, "%d:\t SERF %d:\t boards:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);

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
            --*(mem->barrier);
  
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
                *(mem->barrier)=4;
                sem_post(sem->semWrite);
                sem_post(sem->mutex);
                
            }
            exit(0);
            break;
            

        } else {
            sem_post(sem->accessPier);
            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t SERF %d:\t leaves queue:\t %d:\t %d\n",++(*mem->operationNumber),  ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);

            usleep(rand()%param->returnTime*1000);

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t SERF %d:\t is back\n",++(*mem->operationNumber),ID);
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
    fprintf(output, "%d:\t HACK %d:\t starts\n",++(*mem->operationNumber),ID);
    fflush(output); 
    sem_post(sem->semWrite);

    bool isCaptain = false;

    while(1){
        sem_wait(sem->accessPier);
        if(((*(mem->serfsPier))+(*(mem->hackersPier)))<param->pierCapacity){
           // (*(mem->hackersPier))++;

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t HACK %d:\t waits:\t %d:\t %d\n",++(*mem->operationNumber), ID, ++(*mem->hackersPier), *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);
            sem_post(sem->accessPier);

            sem_wait(sem->mutex);
            if(++(*(mem->hackerBoard))==4){
                
                sem_wait(sem->semWrite);
                *(mem->hackersPier) -=4;
                *(mem->hackerBoard)=0;
                fprintf(output, "%d:\t HACK %d:\t boards:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);

                usleep(rand()%param->cruiseTime*1000);

                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);
                sem_post(sem->hackerQueue);
                isCaptain = true;
            } else if((*(mem->hackerBoard)==2 )&&( *(mem->serfBoard) >=2)){
                
                sem_wait(sem->semWrite);
                *(mem->hackersPier) -=2;
                *(mem->hackerBoard) =0;
                *(mem->serfBoard)-=2;
                *(mem->serfsPier)-=2;
                fprintf(output, "%d:\t HACK %d:\t boards:\t %d:\t %d\n",++(*mem->operationNumber), ID, *mem->hackersPier, *mem->serfsPier);
                fflush(output); 
                sem_post(sem->semWrite);
                usleep(rand()%param->cruiseTime*1000);

                sem_post(sem->serfQueue);
                sem_post(sem->serfQueue);
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
            --*(mem->barrier);
  
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
                *(mem->barrier)=4;
                sem_post(sem->semWrite);
                sem_post(sem->mutex);
            }

            exit(0);
            break;
            
        } else {
            sem_post(sem->accessPier);
            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t HACK %d:\t leaves queue:\t %d:\t %d\n",++(*mem->operationNumber),  ID, *mem->hackersPier, *mem->serfsPier);
            fflush(output); 
            sem_post(sem->semWrite);

            usleep(rand()%param->returnTime*1000);

            sem_wait(sem->semWrite);
            fprintf(output, "%d:\t HACK %d:\t is back\n",++(*mem->operationNumber),ID);
            fflush(output); 
            sem_post(sem->semWrite);

        }
    }
}
