#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 500
#define THREADS 5

int numthreads;
int size;

double **A;
double **C;

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

void generate_input_matrix() {
  srand(42);// for same random matrix every run (to make it easier to test and debug)
  A = malloc(size * sizeof(double*));
  for (int i = 0; i < size; i++) {
    A[i] = malloc(size * sizeof(double));
    for (int j = 0; j < size; j++) {
      A[i][j] = (double)(rand() % 256);// random value between 0 and 255
    }
  }
  
  C = malloc(size * sizeof(double*));
  for (int i = 0; i < size - 2; i++) {
    C[i] = malloc(size * sizeof(double));
  }
  /*
  for(int i=0;i<size;i++){
    for(int j=0;j<size;j++){
      A[i][j] = (double)(rand() % 256);// random value between 0 and 255
      C[i][j] = 0.0;
    }
  }
  
  for (int i = 0; i < size-2; i++) {
    for (int j = 0; j < size -2; j++) {
      C[i][j] = 0.0;
    }
  }
  */
}

void write_input_matrix_to_file(const char *filename) {
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
  
  for(int i=start;i<=end;i++){
    for(int j=1;j<size-1;j++){
      double sum = 0;

      for(int kerni=-1;kerni<=1;kerni++){
        for(int kernj=-1;kernj<=1;kernj++){

          int totali=i+kerni;
          int totalj=j+kernj;

          if(totali>=0 && totali < size && totalj >=0 && totalj < size){
          sum+=A[totali][totalj] * kernel[kerni+1][kernj+1];
          }
        }
      }
      C[i-1][j-1]=sum;
    }
  }

  pthread_barrier_wait(&barrier);
  printf("Thread %d finished convolution\n", data->id);
  return NULL;
}


double calculate_elapsed_time(struct timespec start, struct timespec end) {
  double seconds = (double)(end.tv_sec - start.tv_sec);
  double nanoseconds = (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;

  return seconds + nanoseconds;
}

void write_output_matrix_to_file(const char *filename, double elapsed_time) {
  FILE *fp = fopen(filename, "w");

  if (fp == NULL) {
    perror("Error opening output file");
    return;
  }

  fprintf(fp, "Parallel Matrix Convolution Output\n");
  fprintf(fp, "Matrix size: %d x %d\n", size-2, size-2);
  fprintf(fp, "Number of threads: %d\n", numthreads);
  fprintf(fp, "Execution time: %.6f seconds\n\n", elapsed_time);

  fprintf(fp, "Output Matrix C:\n\n");

  for (int i = 0; i < size-2; i++) {
    for (int j = 0; j < size-2; j++) {
      fprintf(fp, "%8.2f ", C[i][j]);
    }
    fprintf(fp, "\n");
  }

  fclose(fp);
}



int main() {
  pthread_t *threads;
  struct timespec start_time;
  struct timespec end_time;
  double elapsed_time;

  printf("Enter matrix size: ");
  scanf("%d", &size);

  if (size < 3) {
    printf("Invalid size. Please enter a value greater than or equal to 3.\n");
    return 1;
  }
  
  printf("Enter number of threads: ");
  scanf("%d", &numthreads);

  if (numthreads <= 0) {
    printf("Invalid size. Please enter a value greater than 0.\n");
    return 1;
  }

  threads = malloc(numthreads * sizeof(pthread_t));

  generate_input_matrix(size);
  write_input_matrix_to_file("input_matrix.txt");

  printf("Input matrix generated and saved to input_matrix.txt\n");

  clock_gettime(CLOCK_MONOTONIC, &start_time);
  
  pthread_barrier_init(&barrier, NULL, numthreads);
  
  thread_data * thdata;
  thdata = malloc(numthreads * sizeof(thread_data));
  
  int blocksize = (size - 2) / numthreads;
  if (blocksize < 1) blocksize = 1;
  for (int i = 0; i < numthreads; i++) {
    thdata[i].id = i;
    thdata[i].start = i * blocksize + 1;
    thdata[i].end = (i + 1) * blocksize;
    if (thdata[i].end > size - 2) thdata[i].end = size - 2;
  }
  
  for (int i = 0; i < numthreads; i++) {
    pthread_create(&threads[i], NULL, worker, &thdata[i]);
  }
  
  for (int i = 0; i < numthreads; i++) {
    pthread_join(threads[i], NULL);
  }
  
  pthread_barrier_destroy(&barrier);

  clock_gettime(CLOCK_MONOTONIC, &end_time);

  elapsed_time = calculate_elapsed_time(start_time, end_time);

  write_output_matrix_to_file("output_matrix.txt", elapsed_time);

  printf("Convolution complete.\n");
  printf("Output matrix and execution time saved to output_matrix.txt\n");
  printf("Execution time: %.6f seconds\n", elapsed_time);

  return 0;
}
