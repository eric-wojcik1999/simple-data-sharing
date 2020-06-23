#define D 25
#define B 5
void createMemory(int* dFD, int* bFD, int* wincFD, int* wbuffcFD, int* buffEmptyFD, int* rdrcFD, int* semFD,
 int* databuffFD, int*rstoreFD, int* wstoreFD, int numRdr, int numWrt,int*winFD,int*iswrtFD);
void mapMemAddr(int* dFD, int* bFD, int* wincFD, int* wbuffcFD, int* buffEmptyFD, int* rdrcFD, int* semFD, int* databuffFD,
 int*rstoreFD, int* wstoreFD, int numRdr, int numWrt, int** dPT,int** bPT,int** wincPt,int** wbuffcPT,int** buffEmptyPT,
int** rdrcPT,int(**databuffPT),int(**rstorePT),int(**wstorePT),sem_t** sems,int** winPT, int** iswrtPT,int* winFD,int*iswrtFD);
int* readData(int* shared_data, int arrNum);
void wrtRoutine(int pid, int* isWrt, int* wincPT, int* wbuffcPT,int* buffEmptyPT,int* wstorePT, int*databuffPT,
int* shared_data,sem_t* sems,int sleepW);
void rdrRoutine(int pid, int* buffEmptyPT, int* rstorePT, int* databuffPT, sem_t* sems,int* rdrcPT,int sleepR);
int* arrayInit(int* array, int count);
void eraseFile(FILE *simOut);
void writeOut(int pID, int rwNum, int numData, FILE *simOut, char* rw1, char* rw2,char* rw3);
void validCom(int argc, char* argv[]);





