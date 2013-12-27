#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stddef.h>

#include <getpar.h>
#include <birds/birds.h>
#include <glut/mglut.h>

int main(int argc, char * argv[]) {
	int rank, size, i = 0, acc = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

		if(rank == 0)
			glut_non_blocking(argc, argv);

		MPI_Bootstrap(argc, argv, rank);
		
		while(1) {
			MPI_Step(rank, size);
			usleep(FPS);
		}		

	MPI_Finalize();

	pthread_join(glut_thread, NULL);

	return EXIT_SUCCESS;
}