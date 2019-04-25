#include <stdio.h>
#include <stdlib.h>
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

sem_t *serfId = NULL;
sem_t *file_sem = NULL;
sem_t *mutex = NULL;
sem_t *hackerQueue = NULL;
sem_t *serfQueue = NULL;
sem_t *sem = NULL;
sem_t *accessPier = NULL;

FILE *output = NULL;

int SMserfId;
int *serfIdcount = NULL;

int SMserfsPier;
int *serfsPier= NULL;

int SMhackersPier;
int *hackersPier= NULL;

int SMoperationNumber;
int *operationNumber= NULL;

int SMbarrier;
int *barrier= NULL;

int write_to_file(bool person, char *text, int ID);
int set_mem();
int clear_mem();
int close_sem();
int serf_live(int pierCapacity, int returnTime, int cruiseTime);
void serfGenerator(int serfsTime, int personsCount, int pierCapacity, int returnTime, int cruiseTime);

void serfGenerator(int serfsTime, int personsCount, int pierCapacity, int returnTime, int cruiseTime)
{
    pid_t riderPID = 0;
    pid_t *riders = malloc(personsCount*sizeof(pid_t));
    
  // Caka dobu, ktora je urcena na generovanie procesu
    for (int i = 0; i < personsCount; i++){
        if(serfsTime != 0){
        usleep(rand()%serfsTime*1000);
    }
    // Vytvori proces
    riderPID = fork();
    // Ak sa fork() nepodaril
    if(riderPID < 0){
        fprintf(stderr, "Problem with fork\n");
        if (close_sem() != 0) {
            fprintf(stderr, "Problem with closing semaphores\n");
            if (clear_mem() != 0) {
                fprintf(stderr, "Problem with cleaning of the memory\n");
                exit(2);
            }
            exit(2);
        }

      // Vycistenie pamate
        if (clear_mem() != 0) {
            fprintf(stderr, "Problem with cleaning of the memory\n");
            exit(2);
        }
        exit(4);
        kill(0,SIGKILL);

    } else if (riderPID == 0){
      // Spustenie procesu rider
        serf_live(pierCapacity, returnTime, cruiseTime);

    } else {
      // Ulozenie PID procesu, kvoli cakaniu
      riders[i] = riderPID;
    }
  }

  // Caka, kym nie su vsetky procesy ukoncene
  for (int m = 0; m < personsCount; m++){
    if(waitpid(riders[m], NULL, 0) == -1)
      fprintf(stderr,"Can't wait for child\n");
  }
  free(riders);
  exit (0);
}




int set_mem()
{
    int err = 0;
    if ((serfId = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) err = -1;
    if ((mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) err = -1;
    if ((file_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) err = -1;
    if ((hackerQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) err = -1;
    if ((serfQueue = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) err = -1;
    if ((sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) err = -1;
    if ((accessPier = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0)) == MAP_FAILED) err = -1;



       /*
    if((serfId= sem_open("/serfId", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) err = -1; 
    if((mutex = sem_open("/mutex", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) err = -1; 
    if((file_sem= sem_open("/file_sem", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) err = -1; 
    if((hackerQueue = sem_open("/hackerQueue", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) err = -1; 
    if((serfQueue= sem_open("/serfQueue", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) err = -1; 
    if((sem= sem_open("/sem", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) err = -1; 
    if((accessPier= sem_open("/accessPier", O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) err = -1; 
*/
    if (err == 0){
    if (sem_init(serfId, 1, 1) == -1) err = -1;
    if (sem_init(file_sem, 1, 1) == -1) err = -1;
    if (sem_init(mutex, 1, 1) == -1) err = -1;
    if (sem_init(hackerQueue, 1, 1) == -1) err = -1;
    if (sem_init(serfQueue, 1, 0) == -1) err = -1;
    if (sem_init(sem, 1, 0) == -1) err = -1;
    if (sem_init(accessPier, 1, 1) == -1) err = -1;
    }

    if (err == -1) {
        fprintf(stderr, "Problem with creating semaphores\n");
        if (clear_mem() != 0) {
            fprintf(stderr, "Problem with cleaning of the memory\n");
            return -2;
        }
        return -4;
  }

  if((SMserfId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1)
  { return -2; }

  if((serfIdcount = shmat(SMserfId, NULL, 0)) == NULL)
  { return -2; }

  if((SMserfsPier = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1)
  { return -2; }

  if((serfsPier= shmat(SMserfsPier, NULL, 0)) == NULL)
  { return -2; }

  if((SMhackersPier = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1)
  { return -2; }

  if((hackersPier= shmat(SMhackersPier, NULL, 0)) == NULL)
  { return -2; }

    if((SMoperationNumber = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1)
  { return -2; }

  if((operationNumber= shmat(SMoperationNumber, NULL, 0)) == NULL)
  { return -2; }

  if((SMbarrier= shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666)) == -1)
  { return -2; }

  if((barrier= shmat(SMbarrier, NULL, 0)) == NULL)
  { return -2; }

  

  return 0;
}



int clear_mem()
{
  fclose(output);

    if(shmctl(SMserfId, IPC_RMID, NULL) == -1){
        return -2; 
    }
    if(shmctl(SMserfsPier, IPC_RMID, NULL) == -1){
        return -2; 
    }
    if(shmctl(SMhackersPier, IPC_RMID, NULL) == -1){
        return -2; 
    }
    if(shmctl(SMoperationNumber, IPC_RMID, NULL) == -1){
        return -2; 
    }
    if(shmctl(SMbarrier, IPC_RMID, NULL) == -1){
        return -2; 
    }
  return 0;

}



int close_sem()
{
    if(sem_destroy(serfId) != 0) { return -1; }
    if(sem_destroy(file_sem) != 0) { return -1; }
    if(sem_destroy(mutex) != 0) { return -1; }
    if(sem_destroy(hackerQueue) != 0) { return -1; }
    if(sem_destroy(serfQueue) != 0) { return -1; }
    if(sem_destroy(sem) != 0) { return -1; }
    if(sem_destroy(accessPier) != 0) { return -1; }

    munmap(serfId,sizeof(sem_t));
    munmap(file_sem,sizeof(sem_t));
    munmap(mutex,sizeof(sem_t));
    munmap(hackerQueue,sizeof(sem_t));
    munmap(serfQueue,sizeof(sem_t));
    munmap(sem,sizeof(sem_t));
    munmap(accessPier,sizeof(sem_t));
    return 0;
}




int serf_live(int pierCapacity, int returnTime, int cruiseTime)
{
    sem_wait(serfId);
    int ID = (*serfIdcount)++;
    sem_post(serfId);

    sem_wait(file_sem);
    (*operationNumber)++;
    fprintf(output, "%d:\t SERF %d:\t start\n",*operationNumber, ID);
    fflush(output); 
    sem_post(file_sem);


    //write_to_file(false, "start", ID);
    bool isCaptain = false;

    while(1){
        sem_wait(accessPier);
        if(((*serfsPier)+(*hackersPier))<pierCapacity){
            (*serfsPier)++;
            sem_wait(file_sem);
            (*operationNumber)++;
            fprintf(output, "%d:\t SERF %d:\t waits:\t %d:\t %d\n",*operationNumber, ID, *hackersPier, *serfsPier);
            fflush(output); 
            sem_post(file_sem);
            sem_post(accessPier);

          

            sem_wait(mutex);
            if(*serfsPier==4){
                *serfsPier =0;
                sem_wait(file_sem);
                (*operationNumber)++;
                fprintf(output, "%d:\t SERF %d:\t boards:\t %d:\t %d\n",*operationNumber, ID, *hackersPier, *serfsPier);
                fflush(output); 
                sem_post(file_sem);
                usleep(rand()%cruiseTime*1000);

                sem_post(serfQueue);
                sem_post(serfQueue);
                sem_post(serfQueue);
                sem_post(serfQueue);
                isCaptain = true;
            } else if(*serfsPier==2 && *hackersPier >=2){
                *serfsPier =0;
                *hackersPier -=2;
                usleep(rand()%cruiseTime*1000);

                sem_post(serfQueue);
                sem_post(serfQueue);
                sem_post(hackerQueue);
                sem_post(hackerQueue);

                isCaptain = true;
            } else {
                sem_post(mutex);
            }

            sem_wait(serfQueue);


            if (!isCaptain){
                sem_wait(file_sem);
                (*operationNumber)++;
                fprintf(output, "%d:\t SERF %d:\t member exits:\t %d:\t %d\n",*operationNumber, ID, *hackersPier, *serfsPier);
                fflush(output); 
                sem_post(file_sem);
            }


            sem_wait(file_sem);
            (*barrier)--;
  

            if(*barrier == 0){
               
                sem_post(sem);
                sem_post(sem);
                sem_post(sem);
                sem_post(sem);
            }
            sem_post(file_sem);
            sem_wait(sem);
            
             if (isCaptain){
                sem_wait(file_sem);
                (*operationNumber)++;
                fprintf(output, "%d:\t SERF %d:\t captain exits:\t %d:\t %d\n",*operationNumber, ID, *hackersPier, *serfsPier);
                fflush(output);
                *barrier=4;
                sem_post(file_sem);
                sem_post(mutex);
                
            }
                

            /* else {
                sem_wait(sem);
                sem_wait(sem);
                sem_wait(sem);

                sem_wait(file_sem);
                (*operationNumber)++;
                fprintf(output, "%d:\t SERF %d:\t captain exits:\t %d:\t %d\n",*operationNumber, ID, *hackersPier, *serfsPier);
                fflush(output); 
                sem_post(file_sem);
                sem_post(mutex);
            }*/

            break;

        } else {
            sem_post(accessPier);
            sem_wait(file_sem);
            (*operationNumber)++;
            fprintf(output, "%d:\t SERF %d:\t leaves queue:\t %d:\t %d\n",*operationNumber, ID, *hackersPier, *serfsPier);
            fflush(output); 
            sem_post(file_sem);

            usleep(rand()%returnTime*1000);

            sem_wait(file_sem);
            (*operationNumber)++;
            fprintf(output, "%d:\t SERF %d:\t is back:\t %d:\t %d\n",*operationNumber, ID, *hackersPier, *serfsPier);
            fflush(output); 
            sem_post(file_sem);

        }
    }
    exit(0);
}




int main(int argc, char const *argv[]){
    int personsCount, hackersTime, serfsTime, cruiseTime, returnTime, pierCapacity;
    char *stringValue;
    printf("sem\n");

    if (argc != 7){
        fprintf(stderr, "Wrong number of parameters\n");
        return -1;
    }
    else {
        personsCount = strtol(argv[1], &stringValue, 10);
        if (stringValue[0] != '\0') {
            fprintf(stderr, "Type only numbers\n");
            return 1;
        }
        hackersTime = strtol(argv[2], &stringValue, 10);
        if (stringValue[0] != '\0') {
            fprintf(stderr, "Type only numbers\n");
            return 1;
        }
        serfsTime = strtol(argv[3], &stringValue, 10);
        if (stringValue[0] != '\0') {
                fprintf(stderr, "Type only numbers\n");
                return 1;
        }
        cruiseTime = strtol(argv[4], &stringValue, 10);
        if (stringValue[0] != '\0') {
            fprintf(stderr, "Type only numbers\n");
            return 1;
        }
        returnTime = strtol(argv[5], &stringValue, 10);
        if (stringValue[0] != '\0') {
            fprintf(stderr, "Type only numbers\n");
            return 1;
        }
        pierCapacity = strtol(argv[6], &stringValue, 10);
        if (stringValue[0] != '\0') {
            fprintf(stderr, "Type only numbers\n");
            return 1;
        }
    }

    output = fopen("proj2.out", "w+");
    if (output == NULL) {
        return -1;
    }
     setlinebuf(output);
    
    if(set_mem() != 0){
        fprintf(stderr, "Problem with memory allocation\n");
        return -2;
    };

    *operationNumber = 0;
    *serfsPier = 0;
    *hackersPier = 0;
    *serfIdcount= 1;
    *barrier=4;
    /* pid_t serfPID = 0;

    serfPID = fork();
    if(serfPID==0){
        serf_live();
    } else if (serfPID) 
*/  
    serfGenerator(serfsTime, personsCount, pierCapacity, returnTime, cruiseTime);
    
    if (clear_mem() != 0) {
        fprintf(stderr, "Problem with cleaning of the memory\n");
        return -2;
    };
    if (close_sem() != 0) {
        fprintf(stderr, "Problem with closing semaphores\n");
        return -2;
    }
    return 0;
}

int write_to_file(bool person, char *text, int ID){
    //printf("tu");

    sem_wait(file_sem);
    (*operationNumber)++;
    if(person){
         fprintf(output, "%d:\t HACK %d:\t %s:\t %d:\t %d\n",*operationNumber, ID,*text, *hackersPier, *serfsPier);
    } else {
        fprintf(output, "%d:\t SERF %d:\t %s:\t %d:\t %d\n",*operationNumber, ID, *text, *hackersPier, *serfsPier);
    }
    //fflush(output); 
    sem_post(file_sem);
    return 0;
}
