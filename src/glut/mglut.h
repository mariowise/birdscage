#ifndef MGLUT_H_
#define MGLUT_H_

#define FPS 10*1000

int __argc;

char ** __argv;

double rotate_x;

double rotate_y; 

pthread_t glut_thread;

pthread_mutex_t mutex;


void draw(void);

void eventKeys(int, int, int);

void loopez();

void MPI_Bootstrap(int, char **, int);

void MPI_step(int, int);

void * glut_thread_runner(void);

void glut_non_blocking(int, char **);

int glut_main(int argc, char * argv[]);

#endif