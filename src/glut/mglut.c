#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>

#include <glut/mglut.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <mpi.h>
#include <getpar.h>
#include <birds/birds.h>


void draw(void) {
	// Proceso de limpieza de la pantalla
	glClearColor(0.1,0.1,0.1,1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();

	glEnable(GL_PROGRAM_POINT_SIZE_EXT);
    glPointSize(5.0f);

	// Rota cuando el usuario cambie los valores de rotate_x y rotate_y
	glRotatef( rotate_x, 1.0, 0.0, 0.0 );
	glRotatef( rotate_y, 0.0, 1.0, 0.0 );

	// Dibujo del cubo contenedor sin soltar el pincel
	glBegin(GL_LINE_STRIP);
		glColor3f(   0.4,  0.4,  0.4 ); // Color de las líneas del cubo
		glVertex3f(  0.5, -0.5, -0.5 );
		glVertex3f(  0.5, -0.5,  0.5 );
		glVertex3f( -0.5, -0.5,  0.5 );
		glVertex3f( -0.5, -0.5, -0.5 );
		glVertex3f(  0.5, -0.5, -0.5 );
		glVertex3f(  0.5, 0.5, -0.5 );
		glVertex3f(  -0.5, 0.5, -0.5 );
		glVertex3f(  -0.5, -0.5, -0.5 );
		glVertex3f(  -0.5, 0.5, -0.5 );
		glVertex3f(  -0.5, 0.5, 0.5 );
		glVertex3f(  -0.5, -0.5, 0.5 );
		glVertex3f(  -0.5, 0.5, 0.5 );
		glVertex3f(  0.5, 0.5, 0.5 );
		glVertex3f(  0.5, -0.5, 0.5 );
		glVertex3f(  0.5, 0.5, 0.5 );
		glVertex3f(  0.5, 0.5, -0.5 );
	glEnd();

	int i;
	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 1.0f); // Color de las líneas del cubo
	double x, y, z;
	for(i = 0; i < birdsList_size; i++) {
		x = (birdsList[i].pos.x - 500.0f) / 1000.0f;
		y = (birdsList[i].pos.y - 500.0f) / 1000.0f;
		z = (birdsList[i].pos.z - 500.0f) / 1000.0f;
		// printf("GL_POINT (%.1f, %.1f, %.1f)\n", x, y, z);
		glVertex3f(x, z, y);
	}
	glEnd();

	glFlush();
	glutSwapBuffers();
	glFlush();
}

void eventKeys(int key, int x, int y ) {
	#define ROTATESPEED 1
	
	if (key == GLUT_KEY_RIGHT)
		rotate_y += ROTATESPEED;
	else if (key == GLUT_KEY_LEFT)
		rotate_y -= ROTATESPEED;
	else if (key == GLUT_KEY_UP)
		rotate_x += ROTATESPEED;
	else if (key == GLUT_KEY_DOWN)
		rotate_x -= ROTATESPEED;

	glutPostRedisplay();
}

// Esta función se ejecuta siempre que no haya otro evento que atender
void loopez() {
	// printf("Loopez looping the world!\n");
	// rotate_y += 0.1f;
	
	// Llamada a display
	pthread_mutex_lock(&mutex);
	glutPostRedisplay();
	pthread_mutex_unlock(&mutex);
	
	// Periodo de refresco 
	usleep(FPS);
}

void MPI_Bootstrap(int argc, char * argv[], int rank) {
	int i = 0, acc = 0;

	// Obtención de los parámetros de ejecución
	getpar(argc, argv);

	// Configuración de tipos especiales MPI
	birds_buildTypes();

	if(rank == 0) {
		birds_ini(); // Se lee la entrada y se construye 
		birds_show(birdsList); // Se muestra el punto de partida
		// printf("Proceso %d ha iniciado el broadcast inicial.\n", rank);
	} else {
		birdsList = (Bird *) malloc(sizeof(Bird) * par.N);
		birdsListNext = (Bird *) malloc(sizeof(Bird) * par.N);
		
		// El id -1 significa que esta vacío
		for(i = 0; i < par.N; i++)
			birdsList[i].id = -1;
	}

	// Broadcast para que todos los procesos tengan la lista de pájaros
	MPI_Bcast(&birdsList[0], par.N, MPI_Bird, 0, MPI_COMM_WORLD);
	birdsList_size = par.N;
	
	// Se clona la lista en la lista futura
	birds_clone(birdsListNext, birdsList);
	for(i = 0; i < par.N; i++)
		birdsListNext[i].id = -1; // Aun asi en la lista futura no hay nada
	
	// printf("- proceso %d ha terminado el broadcast inicial.\n", rank);

	MPI_Barrier(MPI_COMM_WORLD);
}

void MPI_Step(int rank, int size) {
	int i = 0, acc = 0;
	// Los procesos hacen su pega //////////////////////////////////////////////////////////////////////////
	for(i = 0; i < rank; i++)
		acc += (par.N / size) + ( (i < (par.N % size)) ? 1 : 0 );
	int firstJob = acc;
	int jobsCount = (par.N / size) + ( (rank < (par.N % size)) ? 1 : 0 );
	int currentJob = firstJob;		

		// printf("PID #%d hará %d jobs partiendo desde el %d hasta el %d\n", 
		// 	rank, jobsCount, firstJob, firstJob + jobsCount-1);

	for(i = 0; i < jobsCount; i++) {
		birds_fly(&birdsList[currentJob], &birdsListNext[currentJob]);
		birdsListNext[currentJob].id = currentJob;
		currentJob++;
	}

	// if(rank == 0) {
	// 	printf("Asi esta luciendo despues del primer step\n");
	// 	birds_show(birdsListNext);
	// }

	birds_gather(size);
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(rank == 0) {
		printf("\nResultado despues de gather para proceso #%d\n", rank);
		birds_show(birdsListNext);
	}
	pthread_mutex_lock(&mutex);
	birds_clone(birdsList, birdsListNext);
	pthread_mutex_unlock(&mutex);
	for(i = 0; i < par.N; i++)
		birdsListNext[i].id = -1; // Aun asi en la lista futura no hay nada
}

void * glut_thread_runner(void) {
	printf("Hebra glut creada\n");
	glut_main(__argc, __argv);
}

void glut_non_blocking(int argc, char * argv[]) {
	__argv = argc;
	__argv = argv;
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&glut_thread, NULL, glut_thread_runner, NULL);
}

int glut_main(int argc, char * argv[]) {
	glutInit(&argc, argv);
	rotate_x = -30.0f;
	rotate_y = 35.0f; 
	
	// Simple buffer
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
	glutInitWindowPosition(50,25);
	glutInitWindowSize(600,600);
	glutCreateWindow("Da birds cage");
	glEnable(GL_DEPTH_TEST);
	
	// Llamada a los manejadores de eventos
	glutDisplayFunc(draw);
	glutSpecialFunc(eventKeys);
	glutIdleFunc(loopez);
	
	// Comienza el gluteo
	glutMainLoop();

	return 0;
}
