int* readData(int* shared_data, int arrNum);
void eraseFile(FILE *simOut);
void writeOut(int pID, int rwNum, int numData, FILE *simOut, char* rw1, char* rw2,char* rw3);
void freeFunc(int* wrtStore, int* rdrStore, int* sd, int* db, pthread_t* r, pthread_t* w);
void validCom(int argc, char* argv[]);
int* arrayInit(int* array, int numInc);
typedef struct storage
{
  int sleepR;
  int sleepW;
  int* wrtStore;
  int* rdrStore;
}storage;
