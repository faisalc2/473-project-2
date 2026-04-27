#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 500
#define THREADS 5

double A[N][N];
double C[N][N];

double kernel[3][3] = {
  {0, -1, 0},
  {-1, 5, -1},
  {0, -1, 0}
};

pthread_barrier_t barrier;

typedef struct{
  int id;
  int start;
  int end;
} thread_data;

void* worker(void* arg){
  thread_data *data = (thread_data*)arg;
  
  int start=data->start;
  int end=data->end;

  for(int i=start;i<end;i++){
    for(int j=0;j<N;j++){
      double sum = 0;

      for(int kerni=-1;kerni<=1;kerni++){
        for(int kernj=-1;kernj<=1;kernj++){

          int totali=i+kerni;
          int totalj=j+kernj;

          if(totali>=0 && totali < N && totalj >=0 && totalj < N){
          sum+=A[totali][totalj] * kernel[kerni+1][kernj+1];
          }
        }
      }
    C[i][j]=sum;
    }
  }

  pthread_barrier_wait(&barrier);
  return NULL;
}
