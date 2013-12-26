#ifndef GETPAR_H_
#define GETPAR_H_

struct _par {
	
	int N; 		// Número de aves en la simulación
	int I;		// Número de iteraciones
	char * f;	// Archivo con la configuración inicial

	int check[3];

} par;

void getpar(int argc, char * argv[]);

void printpar();

#endif