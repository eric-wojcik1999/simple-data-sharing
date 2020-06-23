/******************************************************************************
* Author: Eric Wojcik
* Student ID: 19142124
* COMP2006 - Operating Systems Assignment
* Reader-Writer solution (first problem) using pthreads and mutex
* Last Modified: 6/5/2018
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t wrtCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t rdrCond = PTHREAD_COND_INITIALIZER;
int* data_buffer;
int* shared_data;
/*Global varaible used to increment shared_data array specifically for writers*/
int wIndex=0;
/*Increments shared buffer count for writers*/
int curBuff=0;
/*Used to maintain loop in either reader or writer*/
int isWriting=1,isReading=1;
/*D is shared_data size, where as B is buffer size. Req: B<D*/
int D=0,B=0;
int wID=0,rID=0,wrtCnt=0,rdrCnt=0,numRdr=0,numWrt=0,inc=0,bufferEmpty=1;
void *wtrRoutine(void *arg);
void *rdrRoutine(void *arg);

int main(int argc, char* argv[])
{
  int i,err;
  pthread_t* readers;
  pthread_t* writers;
  storage A;
  FILE *simOut=NULL;

  /*Command line validation*/
  validCom(argc,argv);
  numRdr = atoi(argv[1]);
  numWrt = atoi(argv[2]);
  A.sleepR = atoi(argv[3]);
  A.sleepW = atoi(argv[4]);
  D = atoi(argv[5]);
  B = atoi(argv[6]);

  if(B>D)
  {
    printf("B value needs to be greater than D value!\n");
    exit(1);
  }

  /*Setting up*/
  data_buffer = (int*)malloc(sizeof(int)*B);
  shared_data = (int*)malloc(sizeof(int)*D);
  A.wrtStore = (int*)malloc(sizeof(int)*numWrt);
  A.rdrStore = (int*)malloc(sizeof(int)*numRdr);
  readers = (pthread_t*)malloc(sizeof(pthread_t)*numRdr);
  writers = (pthread_t*)malloc(sizeof(pthread_t)*numWrt);
  arrayInit(data_buffer, B);
  arrayInit(shared_data, D);
  arrayInit(A.wrtStore, numWrt);
  arrayInit(A.rdrStore, numRdr);
  eraseFile(simOut);
  readData(shared_data,D);

  /*Creating pthreads for both readers and writers. Upon creation of a thread,
  it will execute a corresponding routine function*/
  for(i=0;i<numRdr;i++)
  {
    if((err=(pthread_create(&readers[i],NULL,rdrRoutine,&A)))!=0)
    {
      fprintf (stderr, "Error = %d (%s)\n", err, strerror (err));
      exit (1);
    }
  }
  for(i=0;i<numWrt;i++)
  {
    if((err=(pthread_create(&writers[i],NULL,wtrRoutine,&A)))!=0)
    {
      fprintf (stderr, "Error = %d (%s)\n", err, strerror (err));
      exit (1);
    }
  }
  for(i=0;i<numRdr;i++)
  {
    pthread_join(readers[i],NULL);
  }
  for(i=0;i<numWrt;i++)
  {
    pthread_join(writers[i],NULL);
  }
  /*Printing results to file "simOut"*/
  for(i=0;i<numWrt;i++)
  {
    writeOut((int)writers[i],i+1,A.wrtStore[i],simOut,"writer","writing","to");
  }
  for(i=0;i<numRdr;i++)
  {
    writeOut((int)readers[i],i+1,A.rdrStore[i],simOut,"reader","reading","from");
  }

  /*Performing cleanup*/
  freeFunc(A.wrtStore, A.rdrStore,shared_data,data_buffer,readers,writers);
  return 0;
}


/******************************************************************************
* Handles routine for any amount of reader threads
* >input:= arg        Void pointer that can take any type
* >output:= NULL
******************************************************************************/
void *rdrRoutine(void *arg)
{
  int pid,buffIndex,rBuffer;
  pid=rID;
  buffIndex=0,rBuffer=0;
  storage *A = arg;
  inc=1;
  while(isReading==1)
  {
    if(bufferEmpty != 1)
    {
      pthread_mutex_lock(&mutex);
      /*Checks if there are any writers waiting to write,writing, or both*/
      if(wrtCnt==1)
      {
        pthread_cond_wait(&rdrCond,&mutex);
      }
      rdrCnt++;
      pthread_mutex_unlock(&mutex);
      rBuffer = buffIndex;
      buffIndex++;
      if(buffIndex==B)
      {
        buffIndex = 0;
      }
      pthread_mutex_lock(&mutex);
      if(inc==D)
      {
        isReading=0;
      }
      else
      {
        /*Reading occurs*/
        printf("Reader %d read data %d\n",pid+1,data_buffer[rBuffer]);
        rdrCnt--;
        A->rdrStore[pid]++;
      }
      if(rdrCnt==0)
      {
        pthread_cond_signal(&wrtCond);
      }
      else
      {
        pthread_cond_signal(&rdrCond);
      }
      inc++;
      pthread_mutex_unlock(&mutex);
      sleep(A->sleepR);
    }
  }
  pthread_exit(NULL);
}

/******************************************************************************
* Handles routine for any amount of reader threads
* >input:= arg        Void pointer that can take any type
* >output:= NULL
******************************************************************************/
void *wtrRoutine(void *arg)
{
  int pid,buffIndex;
  storage *A = arg;
  pid=wID++;
  while(isWriting==1)
  {
    pthread_mutex_lock(&mutex);
    wrtCnt++;
    /*Check if there is writer queued or reader(s) reading*/
    if((wrtCnt>1)||(rdrCnt>0))
    {
      pthread_cond_wait(&wrtCond,&mutex);
    }
    curBuff = buffIndex;
    buffIndex++;
    pthread_mutex_unlock(&mutex);
    pthread_mutex_lock(&mutex);
    if(buffIndex==B)
    {
      buffIndex = 0;
    }
    if(wIndex>=D)
    {
      isWriting=0;
    }
    else
    {
      /*Writing occurs*/
      data_buffer[curBuff]=shared_data[wIndex];
      bufferEmpty = 0;
      printf("Writer %d wrote data %d\n",pid+1,data_buffer[curBuff]);
      A->wrtStore[pid]++;
      wIndex++;
    }
    wrtCnt--;
    if(wrtCnt>0)
    {
      pthread_cond_signal(&wrtCond);
    }
    else
    {
        pthread_cond_broadcast(&rdrCond);
    }
    pthread_mutex_unlock(&mutex);
    sleep(A->sleepW);
  }
  pthread_exit(NULL);
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
******************************************************************************/
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
* Frees any allocated memory and destroys mutexes and conditional variables
* >input:= wrtStore  Contains integer array used to count number of bits written
by a given writer
* >input:= rdrStore  Contains integer array used to count number of bits read
by a given reader
* >input:= sd        Shared_data array
* >input:= db        Data_buffer array
******************************************************************************/
void freeFunc(int* wrtStore, int* rdrStore, int* sd, int* db, pthread_t* r, pthread_t* w)
{
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&wrtCond);
  pthread_cond_destroy(&rdrCond);
  free(r);
  free(w);
  free(wrtStore);
  free(rdrStore);
}

/******************************************************************************
* Validates the command line parameters, make sure there are correct amount
* >input:= argc      Number of command line parameters (including exec name)
* >input:= argv      Char array containing text obtained from command line
******************************************************************************/
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
* Generic array initialisation function that sets all values of array to 0
* >input:= array     Any integer array
* >input:= numInc     Any max value
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
