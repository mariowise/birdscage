#ifndef BIRDS_H_
#define BIRDS_H_

typedef struct {
	double x, y, z;
} Vector3d;

MPI_Datatype MPI_Vector3d;

typedef struct {
	int id;
	Vector3d pos, vel;
} Bird;

MPI_Datatype MPI_Bird;

Bird * birdsList;
Bird * birdsListNext;
int birdsList_size;


void birds_ini(void);

void birds_buildTypes(void);

void birds_show(Bird *);

Vector3d * birds_flyCenter(Bird *);

Vector3d * birds_matchSpeed(Bird *);

Vector3d * birds_keepDistance(Bird *);

void birds_limitSpeed(Bird *);

void birds_limitPosition(Bird *);

void birds_fly(Bird *, Bird *);

void birds_clone(Bird *, Bird *);

void birds_gather(int size);


Vector3d * vector3d_ini(double, double, double);

void vector3d_show(Vector3d, const char *);

double vector3d_abs(Vector3d);

Vector3d * vector3d_sub(Vector3d *, Vector3d, Vector3d);

Vector3d * vector3d_add(Vector3d *, Vector3d, Vector3d, Vector3d, Vector3d);

void vector3d_loadFree(Vector3d *, Vector3d *); 

#endif