/* Ping-pong program */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define NMAX 1000000
#define NREPS 100

char buf[NMAX];

int main(int argc,char *argv[])
{
  int i, n, myid, numprocs;
  double t1, t2;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);

  /* The program takes 1 argument: message size (n), with a default size of 100
     bytes and a maximum size of NMAX bytes*/
  if (argc==2) n = atoi(argv[1]);
  else n = 100;
  if (n<0 || n>NMAX) n=NMAX;

  /* COMPLETE: Get current time, using MPI_Wtime() */
  void* buffer = malloc(n);
  
  t1 = MPI_Wtime();
  for(i = 0; i < NREPS; i++){
    if(myid == 0){
        MPI_Send(buffer, 1, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(buffer, 1, MPI_BYTE, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
    } else if(myid == 1){
        MPI_Recv(buffer, 1, MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(buffer, 1, MPI_BYTE, 0, 1, MPI_COMM_WORLD);   
    }
  }

  /* COMPLETE: Get current time, using MPI_Wtime() */
  t2 = MPI_Wtime();

  /* COMPLETE: Only in process 0.
     Compute the time of transmission of a single message (in milliseconds) and print it.
     Take into account there have been NREPS repetitions, and each repetition involves 2
     messages. */

  

  if(myid == 0){
    /* Printing the computation of NREPS*2 messages because yolo */
    printf("Ping-pong time: %f\n", (t2-t1)/2/NREPS);
    // printf("Ping-pong time: %f\n", (t2-t1)/2/NREPS);
  }

  MPI_Finalize();
  return 0;
}

