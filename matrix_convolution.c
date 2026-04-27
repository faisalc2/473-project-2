#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 500
#define THREADS 5

double A[N][N];
double C[N][N];

double kernel[3][3] = {// given  kernel for convolution
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

void generate_input_matrix(int size) {
  srand(42);// for same random matrix every run (to make it easier to test and debug)

  for(int i=0;i<size;i++){
    for(int j=0;j<size;j++){
      A[i][j] = (double)(rand() % 256);// random value between 0 and 255
      C[i][j] = 0.0;
    }
  }
}

void write_input_matrix_to_file(const char *filename, int size) {
  FILE *fp = fopen(filename, "w");

  if (fp == NULL) {
    perror("Error opening file");
    return;
  }

  fprintf(fp, "Generated Input Matrix A (%d x %d):\n\n", size, size);

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      fprintf(fp, "%.0f ", A[i][j]);
    }
    fprintf(fp, "\n");
  }

  fclose(fp);
}


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


double calculate_elapsed_time(struct timespec start, struct timespec end) {
  double seconds = (double)(end.tv_sec - start.tv_sec);
  double nanoseconds = (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;

  return seconds + nanoseconds;
}

void write_output_matrix_to_file(const char *filename, int size, double elapsed_time) {
  FILE *fp = fopen(filename, "w");

  if (fp == NULL) {
    perror("Error opening output file");
    return;
  }

  fprintf(fp, "Parallel Matrix Convolution Output\n");
  fprintf(fp, "Matrix size: %d x %d\n", size, size);
  fprintf(fp, "Number of threads: %d\n", THREADS);
  fprintf(fp, "Execution time: %.6f seconds\n\n", elapsed_time);

  fprintf(fp, "Output Matrix C:\n\n");

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      fprintf(fp, "%8.2f ", C[i][j]);
    }
    fprintf(fp, "\n");
  }

  fclose(fp);
}



int main() {
  int size;
  struct timespec start_time;
  struct timespec end_time;
  double elapsed_time;

  printf("Enter matrix size: ");
  scanf("%d", &size);

  if (size <= 0 || size > N) {
    printf("Invalid size. Please enter a value between 1 and %d.\n", N);
    return 1;
  }

  generate_input_matrix(size);
  write_input_matrix_to_file("input_matrix.txt", size);

  printf("Input matrix generated and saved to input_matrix.txt\n");

  clock_gettime(CLOCK_MONOTONIC, &start_time);

  /*
    For sheon!!

    1. Initialize the barrier:
       
    2. Create pthread_t array:
     

    3. Create thread_data array:
    

    4. Divide the rows among THREADS.

    5. Use pthread_create() to start worker threads.

    6. Use pthread_join() to wait for all threads to finish.

    7. Destroy the barrier:
  

    IMPORTANT:
    The worker function currently uses N inside the loop.
    If the user enters size = 10, i think the worker should use size, not N.
    
  */

  clock_gettime(CLOCK_MONOTONIC, &end_time);

  elapsed_time = calculate_elapsed_time(start_time, end_time);

  write_output_matrix_to_file("output_matrix.txt", size, elapsed_time);

  printf("Convolution complete.\n");
  printf("Output matrix and execution time saved to output_matrix.txt\n");
  printf("Execution time: %.6f seconds\n", elapsed_time);

  return 0;
}