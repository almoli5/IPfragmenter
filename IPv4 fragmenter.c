/*
IPv4 fragmenter calculator

Takes arguments from command line:
First argument is lenght of datagram
and the rest the MTUs of the consecutive routers, as much as your RAM allows ;)

@Author Luis Rodriguez, Alberto Merino, Álvaro Molinero
*/


//Consider size of datagram lenght in header field - tamaño máximo, lo fragmentaría aunque la mtu fuera mayor

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>


int GetNumberFromString(char *argv);

int FragmentDatagram(int data_datagram, int max_data_mtu, int multiple_8_data_fullfragment, int *offset, int *j, int **fragments, int n, int mf);

int main(int argc, char *argv[])
{
  int i = 0;
  int networks = argc-2;
  int data_length = 0;
  int *mtu;
  int *fragmentsnew = (int*) malloc(sizeof(int));
  int *fragmentsold = (int*) malloc(sizeof(int));
  //For computing
  int max_data_mtu = 0;
  int multiple_8_data_fullfragment = 0;
  int offset = 0;
  int j=0;
  int z=0;
  int n = 1; //number of new fragments
  int N = 1; //number of current fragments
  int mf = 0;

  //Read arguments
  if(argc<3)
  {
    printf("\nError: at least two inputs that are numbers are needed.\n");
    printf("The use of the program is ./nameofprogram datagram_length mtu1 mtu2 mtu3...\n\n");
    free(fragmentsold);
    free(fragmentsnew);
    return 0;
  }
  data_length = GetNumberFromString(argv[1]);//Same as argv[1][0]
  if(data_length <21)
  {
    printf("\nError: datagram lenght is not at least 21, which means it doesn't have a full header and at least 1 byte of info.\n");
    free(fragmentsold);
    free(fragmentsnew);
    return 0;
  }
  fragmentsold[0] = data_length-20;
  mtu = (int *) malloc(networks*sizeof(int));
  printf("\nInput: {\"datagram_length\": %d\n", data_length);
  printf("\"MTU's\":[");
  for (i=0; i < networks; i++)
  {
    mtu[i] = GetNumberFromString(argv[i+2]);
    if(mtu[i]==0)
    {
      free(fragmentsold);
      free(fragmentsnew);
      free(mtu);
      return 0;
    }
    if(mtu[i]<21)
    {
      printf("\nError: at least one mtu is less than 21, which means it can't carry a normal datagram header and at least 1 byte of data.\n");
      return 0;
    }
    printf("%d",mtu[i]);
    if(i < networks-1)
      printf(", ");
  }
  printf("]}");

  //Calculating fragments

  printf("\n\nOutput: {");

  for(i=0; i<networks; i++)
  {
    printf("\"%d\": [",mtu[i]);
    mtu[i] = (mtu[i]>65535)?65535:mtu[i];
    max_data_mtu = mtu[i]-20;
    if(max_data_mtu%8==0)
    {
      multiple_8_data_fullfragment = max_data_mtu;
    } else {
      multiple_8_data_fullfragment = ((int)(max_data_mtu/8))*8;
    }

    j=0;
    offset = 0;
    mf=1;

    for(z=0; z<N ;z++)
    {
      if(z==N-1) mf=0;
      //printf("\nz=%d, size is %d\n", z, fragmentsold[z]);
      n = FragmentDatagram(fragmentsold[z], max_data_mtu, multiple_8_data_fullfragment, &offset, &j, &fragmentsnew, n, mf);
      if(n==0)
      {
        free(mtu);
        free(fragmentsold);
        free(fragmentsnew);
        return 0;
      }
      if(z!=N-1) printf("\n");
    }
    printf("],\n");
    N = n;
    free(fragmentsold);
    fragmentsold = (int*) malloc(sizeof(int)*N);
    for(z=0; z<N ;z++)
    {
      //printf("\nz = %d, fragmentsnew is=%d\n", z, fragmentsnew[z]);
      fragmentsold[z] = fragmentsnew[z];
    }
  }

  free(mtu);
  free(fragmentsold);
  free(fragmentsnew);
  return 0;
}

int GetNumberFromString(char *argv)
{
  int i=0;
  int number = 0;
  int length = 0;

  for(i=0; argv[i]!='\0';i++);

  length = i;

  for(i=0; argv[i]!= '\0';i++)
  {
    if(isdigit(argv[i])!=0)
    {
      number += pow(10,length-i-1)*(argv[i]-'0');
     } else
     {
       printf("\nError: All inputs are not numbers.\n");
       return 0;
     }
  }

  return number;
}

int FragmentDatagram(int data_datagram, int max_data_mtu, int multiple_8_data_fullfragment, int *offset, int *j, int **fragments, int n, int mf)
{
  int j_interno=0;
  for(; (data_datagram - multiple_8_data_fullfragment*(*j)) > max_data_mtu; (*j)++)
  {
    if(multiple_8_data_fullfragment < 8)
    {
      printf("\nDatagram dropped because can't be fragmented since mtu size doesn't have a positive multiple of 8.\n\n");
      return 0;
    }
    n++;
    *fragments = (int *) realloc(*fragments, sizeof(int)*n);
    (*fragments)[*j] = multiple_8_data_fullfragment;
    printf("{\"datagram_length\": %d, \"MF\": 1, \"Offset\": %d}\n",(*fragments)[*j]+20, *offset);
    *offset += (*fragments)[*j]/8;
    if(*offset > 8191)
    {
      printf("\nDatagram dropped because of overflow in offset header field.\n\n");
      return 0;
    }
    j_interno++;
  }
  /*
  Next lines will work both if mf is 1 or 0. If it's 0, the last fragment doesn't need to be a multiple of 8.
  If it's 1, the full datagram is a multiple of 8 so if we take out multiples of 8 we still have a multiple of 8.
  */
  (*fragments)[*j] = (data_datagram-(multiple_8_data_fullfragment*(j_interno)));
  printf("{\"datagram_length\": %d, \"MF\": %d,  \"Offset\": %d}",(*fragments)[*j]+20, mf,*offset);

  *offset += (*fragments)[*j]/8;
  if(*offset > 8191)
  {
    printf("\nDatagram dropped because of overflow in offset header field.\n\n");
    return 0;
  }
  (*j)++;
  return n;
}
