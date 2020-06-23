#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 /******************************************************************************
 * Read the contents of the input file 'shared_data into a shared_data integer
 * array, which is accesible soley by the writer.
 * >input:= shared_data   Integer array to read data into
 * >input:= arrNum        Size of shared_data file (D)
 * >output:= shared_data
 ********************************************************************************/
int* readData(int* shared_data, int arrNum)
{
  int count, n;
  FILE *input = NULL;
  input = fopen("shared_data", "r");
  count = 0;
  if (input==NULL)
  {
    perror("File could not be opened");
    exit(1);
  }

  /*Reads an individual int and scans to a temp int 'n'. Stores 'n' in
    corresponding element position in shared_data array. Repeats until all read*/
  while(fscanf(input, " %d", &n)==1)
  {
    if(count<=arrNum)
    {
      shared_data[count]=n;
    }
    count++;
  }

  if(ferror(input))
  {
    perror("File could not be read");
    exit(1);
  }

  fclose(input); /*Closes the file*/
  return shared_data;
}

