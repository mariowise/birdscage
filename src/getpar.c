#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <getpar.h>

// Obtiene los parámetros con getopt
void getpar(int argc, char * argv[]) {
	int opt, i, flag = 0;
	
	// Punto de partida para la recepción de parámetros
	for(i = 0; i < 3; i++)
		par.check[i] = 0;
	
	while ((opt = getopt(argc, argv, "N:I:f:")) != -1) {
		switch (opt) {
			case 'N':
				par.N = atoi(optarg);
				par.check[0] = 1;
				break;
			case 'I':
				par.I = atoi(optarg);
				par.check[1] = 1;
				break;
			case 'f':
				par.f = (char *) malloc(sizeof(char) * strlen(optarg));
				strcpy(par.f, optarg);
				par.check[2] = 1;
				break;
			default:
				fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	// Verificación de ingreso de parámetros
	for(i = 0; i < 3; i++) {
		if(par.check[i] != 1) {
			flag = 1;
			switch(i) {
				case 0:
					printf("* Error: el parámetro 'N' es obligatorio y no fue proporcionado.\n");
					break;
				case 1:
					printf("* Error: el parámetro 'I' es obligatorio y no fue proporcionado.\n");
					break;
				case 2:
					printf("* Error: el parámetro 'f' es obligatorio y no fue proporcionado.\n");
					break;
			}
		}
	}

	if(flag == 1)
		exit(0);
	
	if (optind != argc) {
		fprintf(stderr, "Expected argument after options %d != %d\n", optind, argc);
		exit(EXIT_FAILURE);
	}
}

// Muestra los parámetros ingresados en la consola
void printpar() {
	printf(" -N: %d\n", par.N);
	printf(" -I: %d\n", par.I);
	printf(" -f: %s\n", par.f);
}
