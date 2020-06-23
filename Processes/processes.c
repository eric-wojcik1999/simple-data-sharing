/******************************************************************************
* Author: Eric Wojcik
* Student ID: 19142124
* COMP2006 - Operating Systems Assignment
* Reader-Writer solution (first problem) using processes and semaphores
* Last Modified: 6/5/2018
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "processes.h"
#include <sys/wait.h>

int main(int argc, char* argv[])
{
  int i,numRdr,numWrt,sleepR,sleepW;
  int* shared_data;
  int* wrtPIDS;
  int* rdrPIDS;
  FILE *simOut=NULL;

  /*File Descriptors*/
  int dFD,bFD,wincFD,wbuffcFD,buffEmptyFD,rdrcFD,semFD,databuffFD,rstoreFD,wstoreFD,winFD,iswrtFD;
  /*Pointers for shared memory*/
  int *dPT,*bPT,*wincPT,*wbuffcPT,*buffEmptyPT,*rdrcPT,*databuffPT,*rstorePT,*wstorePT,*winPT,*iswrtPT;  
  /*Mutex for readCount, mutex for resource and pointer to shared sem space*/
  sem_t mutex,wrt,*sems;
  pid_t pid; 
  
  /*Command line validation*/
  validCom(argc,argv); 
   
  numRdr = atoi(argv[1]);
  numWrt = atoi(argv[2]);
  sleepR = atoi(argv[3]);
  sleepW = atoi(argv[4]);
     
  /*Creating shared memory regions and allocating the sizes for them*/
  createMemory(&dFD,&bFD,&wincFD,&wbuffcFD,&buffEmptyFD,&rdrcFD,&semFD,&databuffFD,&rstoreFD,&wstoreFD,numRdr,numWrt,&winFD,&iswrtFD);
  /*Mapping pointers to the shared memory regions*/
  mapMemAddr(&dFD,&bFD,&wincFD,&wbuffcFD,&buffEmptyFD,&rdrcFD,&semFD,&databuffFD,&rstoreFD,&wstoreFD,numRdr,numWrt,
&dPT,&bPT,&wincPT,&wbuffcPT,&buffEmptyPT,&rdrcPT,&databuffPT,&rstorePT,&wstorePT,&sems,&winPT,&iswrtPT,&winFD,&iswrtFD); 
  
  /*Initialise sempahores with non-zero value meaning it can be shared between processes*/
  if((sem_init(&mutex,1,1)==1)||(sem_init(&wrt,1,1)==1))
  {
    perror("Semaphore initialisation failed");
    exit(1);
  } 

  /*Allocating sempahores to shared memory space via the sems array*/
  sems[0]=mutex;
  sems[1]=wrt;
  
  /*Remaining set-up*/
  shared_data = (int*)malloc(sizeof(int)*D);
  wrtPIDS = (int*)malloc(sizeof(int)*numWrt);
  rdrPIDS = (int*)malloc(sizeof(int)*numRdr);
  *winPT = 0;
  *iswrtPT= 1; 
  *wbuffcPT = 0;
  *buffEmptyPT=0;
  *rdrcPT=0;
  shared_data = arrayInit(shared_data,D);
  wstorePT = arrayInit(wstorePT,numWrt);
  rstorePT = arrayInit(rstorePT,numRdr);
  wrtPIDS = arrayInit(wrtPIDS,numWrt);
  rdrPIDS = arrayInit(rdrPIDS,numRdr); 
  readData(shared_data,D);     
  eraseFile(simOut); 
 
  /*Creating reader and writer processes*/
  for(i=0;i<numWrt;i++)
  {   
    if((pid=fork())==0)
    { 
      wrtRoutine(i,iswrtPT,winPT,wbuffcPT,buffEmptyPT,wstorePT,databuffPT,shared_data,sems,sleepW);           
      exit(0); 
    } 
    else if(pid<0)
    {
      perror("Could not create child process!");
    }    
  }
  for(i=0;i<numRdr;i++)
  {
    if((pid=fork())==0)
    { 
      rdrRoutine(i,buffEmptyPT,rstorePT,databuffPT,sems,rdrcPT,sleepR);
      exit(0);
    }
    else if(pid<0)
    {
      perror("Could not create child process!");
    }   
  }
  /*Parent waiting for all child processes to finish to avoid zombie processes*/
  while((pid=wait(0))>0);
  /*Processes Complete*/
  for(i=0;i<numWrt;i++)
  {  
    writeOut(0,i+1,wstorePT[i],simOut,"writer","writing","to");
  }
  for(i=0;i<numRdr;i++)
  {
    writeOut(0,i+1,rstorePT[i],simOut,"reader","reading","from");
  }
  /*Cleaning up process*/ 
  if(pid>0)
  {
    /*Cleaning up semaphores*/
    sem_close(&(sems[0]));
    sem_close(&(sems[1]));
    sem_destroy(&(sems[0]));
    sem_destroy(&(sems[1]));
    
    /*Cleaning up file descriptors*/
    close(dFD);
    close(bFD);
    close(wincFD);
    close(wbuffcFD);
    close(buffEmptyFD);
    close(rdrcFD);
    close(semFD);
    close(databuffFD);
    close(rstoreFD);
    close(wstoreFD);
    close(iswrtFD);

    /*Clearing out the shared memory regions*/
    shm_unlink("/D_val");
    shm_unlink("/B_val");
    shm_unlink("/winnFD");
    shm_unlink("/iswrting");
    shm_unlink("wincFD");
    shm_unlink("/wrt_buffcnt");
    shm_unlink("/buff_empty");
    shm_unlink("/read_cnt");
    shm_unlink("/sems");
    shm_unlink("/db");
    shm_unlink("/rdr_store");
    shm_unlink("/wrt_store");
  
    /*Clearing out mapped memory regions*/
    munmap(dPT,sizeof(int));
    munmap(bPT, sizeof(int));
    munmap(wincPT, sizeof(int));
    munmap(iswrtPT, sizeof(int));
    munmap(wbuffcPT, sizeof(int));
    munmap(buffEmptyPT, sizeof(int));
    munmap(rdrcPT, sizeof(int));
    munmap(sems, sizeof(sem_t)*2);
    munmap(databuffPT, sizeof(int)*B);
    munmap(rstorePT, sizeof(int)*numRdr);
    munmap(wstorePT, sizeof(int)*numWrt);   
  }
  free(shared_data);
  free(wrtPIDS);
  free(rdrPIDS);
  return 0;
}


/******************************************************************************
* Generic array initialisation function that sets all values of array to 0
* >input:= array     Any integer array
* >input:= numInc    Any max value
* >output:= array
******************************************************************************/
int* arrayInit(int* array, int numInc)
{
  int i;
  for(i=0;i<numInc;i++)
  {
    array[i]=0;
  }
  return array;
}

/******************************************************************************
 * Validates the command line parameters, make sure there are correct amount
 * >input:= argc      Number of command line parameters (including exec name)
 * >input:= argv      Char array containing text obtained from command line
 * ******************************************************************************/
void validCom(int argc, char* argv[])
{
  int i;
  if(argc<5)
  {
    printf("Too few arguments. Need to have r,w,t1,t2,d,b\n");
    exit(1);
  }
  else if(argc>7)
  {
    printf("Too many arguments. Need to have r,w,t1,t2,d,b\n");
    exit(1);
  }
  for(i=1;i<5;i++)
  {
    if(atoi(argv[i])<0)
    {
      printf("Argument %d needs to be a positive value\n",i);
      exit(1);
    }
  }
}


/******************************************************************************
* Handles routine for any amount of reader processes
* >input:= pid            Numerical identifier for current reader
* >input:= buffEmptyPT    Pointer to variable used to indicate if buffer is empty 
* >input:= rstorePT       Pointer to array that stores the amount of bits read by any given reader 
* >input:= databuffPT     Pointer to array that serves as the shared buffer 
* >input:= sems           Array of semaphores, contains semaphores wrt and mutex
* >output:= NULL
******************************************************************************/
void rdrRoutine(int pid, int* buffEmptyPT, int* rstorePT, int* databuffPT, sem_t* sems,int* readCount,int sleepR)
{
  int i,isReading,bufferIndex,curIndex,rinc,localRCnt;
  isReading = 1,bufferIndex=0,curIndex=0,rinc=0,localRCnt=0;
  for(i=0;i<D;i++)
  { 
    if(isReading==1)
    {  
      sem_wait(&(sems[0]));
      *readCount = *readCount + 1; 
      /*If first reader will lock out writer*/ 
      if(*readCount==1)
      {
        sem_wait(&(sems[1]));
      }
      sem_post(&(sems[0]));
      /*Releases mutex to allow another reader to read at same time*/
      curIndex=bufferIndex;
      bufferIndex++;
      if(bufferIndex==B)
      {
        bufferIndex=0;
      }
      if(rinc==D)
      {
        isReading = 0;
        localRCnt=0;
      } 
      else
      {
         printf("Reader %d reading %d from data_buffer index pos %d\n",pid+1,databuffPT[curIndex],curIndex);
         localRCnt=1;
      }
      sem_wait(&(sems[0]));
      rstorePT[pid]=rstorePT[pid]+localRCnt;
      *readCount = *readCount-1;
      /*Checks if the last reader is done, will then unlock writer*/
      if(*readCount==0)
      {
        sem_post(&(sems[1]));
      } 
      rinc++;
      sem_post(&(sems[0]));
      sleep(sleepR); 
    }
  }
}


/******************************************************************************
* Handles routine for any amount of writer processes
* >input:= pid            Numerical identifier for current writer
* >input:= stop           This variable determines when a writer needs to stop writing
* >input:= wincPT         Variable used to increment through the shared_data array. This is in shared memory so that writers can share the load
* >input:= buffEmptyPT    Pointer to variable used to indicate if buffer is empty 
* >input:= wstorePT       Pointer to array that stores the amount of bits written by any given writer 
* >input:= databuffPT     Pointer to array that serves as the shared buffer 
* >input:= shared_data    Array of ints, stores shared_data read in from the shared_data file. Contents are written to data_buffer 
* >input:= sems           Array of semaphores, contains semaphores wrt and mutex
* >output:= NULL
******************************************************************************/
void wrtRoutine(int pid, int* stop, int* wincPT, int* wbuffcPT,int* buffEmptyPT,int* wstorePT, int* databuffPT,
int* shared_data,sem_t* sems,int sleepW)
{
  int i,bufferIndex,localWCnt;
  bufferIndex=0,localWCnt=0;
  for(i=0;i<D;i++)
  { 
    if(*stop==1)
    { 
      sem_wait(&(sems[1]));
      bufferIndex = *wbuffcPT;
      *wbuffcPT=*wbuffcPT+1;
      if(*wbuffcPT==B-1)
      {
        *wbuffcPT=0;
      }

      if(*wincPT==D)
      {
        *stop = 0;
        localWCnt=0; 
      }
      else
      {
        printf("Writer %d has written %d to data_buffer pos %d\n",pid+1,shared_data[*wincPT],bufferIndex);
        databuffPT[bufferIndex]=shared_data[*wincPT];
        *buffEmptyPT=0;
        localWCnt=1;          
      }
      *wincPT=*wincPT+1;
      wstorePT[pid]=wstorePT[pid]+localWCnt;
      sem_post(&(sems[1]));
      sleep(sleepW);
    }
  }
}

/******************************************************************************
 * Clears file so that it is ready for use
 * >input:= simOut   Pointer to FILE simOut
 ******************************************************************************/
void eraseFile(FILE *simOut)
{
  simOut = fopen("sim_out", "w");
  fclose(simOut);
} 

/******************************************************************************
 * Generic function that writes out results of both readers OR writers to a file
 * called 'sim_out'
 * >input:= pID        Thread id number
 * >input:= rwNum      Current reader/writer number
 * >input:= numData    Pieces of data written or read
 * >input:= simOut     Pointer to FILE simOut
 * >input:= rw1        Phrase 'reader' or 'writer'
 * >input:= rw2        Phrase 'read' or 'written'
 * >input:= rw3        Phrase 'from' or 'to'
 * >output:= shared_data
 * ******************************************************************************/
void writeOut(int pID, int rwNum, int numData, FILE *simOut, char* rw1, char* rw2,char* rw3)
{
  simOut = fopen("sim_out", "a");
  if(simOut!=NULL)
  {
    fprintf(simOut,"%s-%d [pid:%d] has finished %s %d pieces of data %s the data-buffer\n",rw1,rwNum,pID,rw2,numData,rw3);
  }
  else /*Check if file was opened properly*/
  {
    perror("Error ");
    exit(1);
  }
  fclose(simOut);
}

/******************************************************************************
 * Maps each supplied pointer to a memory address via the use of a file descriptor. 
 * >input:= dFD            File descriptor for variable D (size of shared_data)
 * >input:= bFD            File descriptor for variable B (size of data_buffer)
 * >input:= wincFD         File descriptor for writer increment variable used to go through shared_data 
 * >input:= wbuffcFD       File descriptor for variable used to increment through buffer (allows writers to share load)
 * >input:= buffEmptyFD    File descriptor for variable used to determine if buffer empty
 * >input:= rdrcFD         File descriptor for variable read count 
 * >input:= semFD          File descriptor for semaphores
 * >input:= databuffFD     File descriptor for data_buffer
 * >input:= rstoreFD       File descriptor for reader array storage (for amount read)
 * >input:= wstoreFD       File descriptor for writer array storage (for amount written)
 * >input:= winFD          File descriptor for writer variable win
 * >input:= iswrtFD        File descriptor for is writing 
 * >input:= numRdr         Integer representing number of readers 
 * >input:= numWrt         Integer representing number of writers 
 * >input:= dPT            Points to new memory address for variable D
 * >input:= bPT            Points to new memory address for variable B
 * >input:= wincPT         Points to new memory address for variable writer increment
 * >input:= wbuffcPT       Points to new memory address for variable writer buffer increment 
 * >input:= buffEmptyPT    Points to new memory address for variable empty buffer
 * >input:= rdrcPT         Points to new memory address for variable reader count 
 * >input:= databuffPT     Points to new memory address for data_buffer array
 * >input:= rstorePT       Points to new memory address for reader storage array
 * >input:= wstorePT       Points to new memory address for writer storage array
 * >input:= sems           Points to new memory address for semaphore storage array
 * >input:= winPT          Points to new memory address for variable writer increment
 * >input:= iswrtPT        Points to new memory address for variable is writing 
 ******************************************************************************/
void mapMemAddr(int* dFD, int* bFD, int* wincFD, int* wbuffcFD, int* buffEmptyFD, int* rdrcFD, int* semFD, int* databuffFD, 
int*rstoreFD, int* wstoreFD, int numRdr, int numWrt, int** dPT,int** bPT,int** wincPT,int** wbuffcPT,int** buffEmptyPT,
int** rdrcPT,int(**databuffPT),int(**rstorePT),int(**wstorePT),sem_t** sems,int** winPT,int** iswrtPT,int* winFD, int* iswrtFD)
{
  *dPT = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *dFD, 0);   
  *bPT = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *bFD, 0);
  *wincPT = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *wincFD, 0);    
  *winPT = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *winFD, 0);
  *iswrtPT = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *iswrtFD, 0);
  *wbuffcPT = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *wbuffcFD, 0);   
  *buffEmptyPT = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *buffEmptyFD, 0);    
  *rdrcPT = (int*) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, *rdrcFD, 0);      
  *sems = mmap(NULL, sizeof(sem_t)*2, PROT_READ | PROT_WRITE, MAP_SHARED, *semFD, 0);
  *databuffPT = (int*) mmap(NULL, sizeof(int)*B, PROT_READ | PROT_WRITE, MAP_SHARED, *databuffFD, 0);
  *rstorePT = (int*) mmap(NULL, sizeof(int)*numRdr, PROT_READ | PROT_WRITE, MAP_SHARED, *rstoreFD, 0);
  *wstorePT = (int*) mmap(NULL, sizeof(int)*numWrt, PROT_READ | PROT_WRITE, MAP_SHARED, *wstoreFD, 0);

  if((*dPT==MAP_FAILED)||(*bPT==MAP_FAILED)||(*wincPT==MAP_FAILED)||(*wbuffcPT==MAP_FAILED)||(*buffEmptyPT==MAP_FAILED)||
  (*rdrcPT==MAP_FAILED)||(*sems==MAP_FAILED)||(databuffPT==MAP_FAILED)||(rstorePT==MAP_FAILED)||(wstorePT==MAP_FAILED)
  ||(*winPT==MAP_FAILED)||(*iswrtPT==MAP_FAILED))
  {
    perror("Failed to map to memory space");
    exit(1);
  }
}



/******************************************************************************
 * Creates a shared memory region for each file descriptor so that they can be accessed by 
 * all processes and child processes.  
 * >input:= dFD            File descriptor for variable D (size of shared_data)
 * >input:= bFD            File descriptor for variable B (size of data_buffer)
 * >input:= wincFD         File descriptor for writer increment variable used to go through shared_data 
 * >input:= wbuffcFD       File descriptor for variable used to increment through buffer (allows writers to share load)
 * >input:= buffEmptyFD    File descriptor for variable used to determine if buffer empty
 * >input:= rdrcFD         File descriptor for variable read count 
 * >input:= semFD          File descriptor for semaphores
 * >input:= databuffFD     File descriptor for data_buffer
 * >input:= rstoreFD       File descriptor for reader array storage (for amount read)
 * >input:= wstoreFD       File descriptor for writer array storage (for amount written)
 * >input:= winFD          File descriptor for writer variable win
 * >input:= iswrtFD        File descriptor for is writing 
 * >input:= numRdr         Integer representing number of readers 
 * >input:= numWrt         Integer representing number of writers 
 ******************************************************************************/
void createMemory(int* dFD, int* bFD, int* wincFD, int* wbuffcFD, int* buffEmptyFD, int* rdrcFD, int* semFD, int* databuffFD,
 int*rstoreFD, int* wstoreFD, int numRdr, int numWrt,int* winFD,int*iswrtFD)
{
 
  /*Creating shared memory region for every variable/struture*/
  *dFD = shm_open("/D_val", O_CREAT | O_RDWR,0666); 
  *bFD = shm_open("/B_val", O_CREAT | O_RDWR,0666);
  *winFD = shm_open("/winnFD", O_CREAT | O_RDWR,0666);
  *iswrtFD = shm_open("/iswrting", O_CREAT | O_RDWR,0666);
  *wincFD = shm_open("wincFD", O_CREAT | O_RDWR,0666);
  *wbuffcFD = shm_open("/wrt_buffcnt", O_CREAT | O_RDWR,0666);
  *buffEmptyFD = shm_open("/buff_empty", O_CREAT | O_RDWR,0666);
  *rdrcFD = shm_open("/read_cnt", O_CREAT | O_RDWR, 0666);
  *semFD = shm_open("/sems", O_CREAT | O_RDWR, 0666);
  *databuffFD = shm_open("/db", O_CREAT | O_RDWR,0666);
  *rstoreFD = shm_open("/rdr_store", O_CREAT | O_RDWR,0666);
  *wstoreFD = shm_open("/wrt_store", O_CREAT | O_RDWR,0666);

  if((*dFD==-1)||(*bFD==-1)||(*wincFD==-1)||(*wbuffcFD==-1)||(*buffEmptyFD==-1)||(*rdrcFD==-1)||(*semFD==-1)||(*databuffFD==-1)
||(*rstoreFD==-1)||(*wstoreFD==-1)||(*winFD==-1)||(*iswrtFD==-1))
  {
    perror("Failed to create memory space ");
    exit(1);
  }
  /*Set size of shared memory objects*/
  if(ftruncate(*dFD, sizeof(int))==-1)
  {
    perror("Failed to configure size for D");
    exit(1);
  }
  if(ftruncate(*bFD, sizeof(int))==-1)
  {
    perror("Failed to configure size for B");
    exit(1);
  }
  if(ftruncate(*wincFD, sizeof(int))==-1)
  {
    perror("Failed to configure size for writer inc");
  }
  if(ftruncate(*winFD, sizeof(int))==-1)
  {
    perror("Failed to configure size for writer inc");
    exit(1);
  }
  if(ftruncate(*iswrtFD, sizeof(int))==-1)
  {
    perror("Failed to configure size of writer stop value");
    exit(1);
  }
  if(ftruncate(*wbuffcFD, sizeof(int))==-1)
  {
    perror("Failed to configure size for writer buffer count");
    exit(1);
  }
  if(ftruncate(*buffEmptyFD, sizeof(int))==-1)
  {
    perror("Failed to configure size for buffer empty");
    exit(1);
  }
  if(ftruncate(*semFD, sizeof(sem_t)*2)==-1)
  { 
    perror("Failed to configure size for mutex and wrt semaphores");
    exit(1);
  }
  if(ftruncate(*rdrcFD, sizeof(int))==-1)
  {
    perror("Failed to configure size for reader  count");
    exit(1);
  }
  if(ftruncate(*databuffFD, sizeof(int)*D)==-1)
  {
    perror("Failed to configure size for data_buffer");
    exit(1);
  }
  if(ftruncate(*rstoreFD, sizeof(int)*numRdr)==-1)
  {
    perror("Failed to configure size for reader data count array");
    exit(1);
  }
  if(ftruncate(*wstoreFD, sizeof(int)*numWrt)==-1)
  {
    perror("Failed to configure size for writer data count");
    exit(1);
  }
}
