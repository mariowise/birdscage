#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include <getpar.h>
#include <birds/birds.h>

void birds_ini(void) {
	FILE * fd;
	char * line = (char *) malloc(sizeof(char));
	size_t len;
	ssize_t read;

	if((fd = fopen(par.f, "r")) == NULL) {
		printf("* Error: I can't open the file '%s'.\n", par.f);
		exit(1);
	}

	char * token;
	birdsList_size = 0;
	birdsList = (Bird *) malloc(sizeof(Bird) * par.N);
	birdsListNext = (Bird *) malloc(sizeof(Bird) * par.N);
	int i = 0;
	while((read = getline(&line, &len, fd)) != -1) {
		birdsList[i].id = i;

		token = strtok(line, " ");
		birdsList[i].pos.x = (double) atof(token);

		token = strtok(NULL, " ");
		birdsList[i].pos.y = (double) atof(token);
		
		token = strtok(NULL, " ");
		birdsList[i].pos.z = (double) atof(token);


		token = strtok(NULL, " ");
		birdsList[i].vel.x = (double) atof(token);

		token = strtok(NULL, " ");
		birdsList[i].vel.y = (double) atof(token);

		token = strtok(NULL, " ");
		birdsList[i].vel.z = (double) atof(token);

		i++;
	}
	birdsList_size = i;

	fclose(fd);
}

void birds_buildTypes(void) {
	// MPI Struct para Vector3d
	int nroItems = 3;
	int blockLengths[3] = { 1, 1, 1 };
	MPI_Datatype types[3] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE };
	MPI_Aint     offsets[3];

	offsets[0] = offsetof(Vector3d, x);
	offsets[1] = offsetof(Vector3d, y);
	offsets[2] = offsetof(Vector3d, z);

	MPI_Type_create_struct(nroItems, blockLengths, offsets, types, &MPI_Vector3d);
	MPI_Type_commit(&MPI_Vector3d);

	    // MPI Struct para Bird
	MPI_Datatype types2[3] = { MPI_INT, MPI_Vector3d, MPI_Vector3d };
	MPI_Aint 	 offsets2[3];

	offsets2[0] = offsetof(Bird, id);
	offsets2[1] = offsetof(Bird, pos);
	offsets2[2] = offsetof(Bird, vel);

	MPI_Type_create_struct(nroItems, blockLengths, offsets2, types2, &MPI_Bird);
	MPI_Type_commit(&MPI_Bird);
}

void birds_show(Bird * list) {
	int i;
	for(i = 0; i < birdsList_size; i++) {
		printf("Bird#%d: pos(%.4f, %.4f, %.4f) vel(%.4f, %.4f, %.4f)\n", list[i].id,
			list[i].pos.x,
			list[i].pos.y,
			list[i].pos.z,
			list[i].vel.x,
			list[i].vel.y,
			list[i].vel.z );
	}
}

Vector3d * birds_flyCenter(Bird * brd) {
	Vector3d * sum = vector3d_ini(0.0f, 0.0f, 0.0f);
	int i;
	for(i = 0; i < birdsList_size; i++) {
		if(brd->id != birdsList[i].id) {
			sum->x += birdsList[i].pos.x / (double) (birdsList_size-1);
			sum->y += birdsList[i].pos.y / (double) (birdsList_size-1);
			sum->z += birdsList[i].pos.z / (double) (birdsList_size-1);
		}
	}
	sum->x = (sum->x - brd->pos.x) / 100.0f;
	sum->y = (sum->y - brd->pos.y) / 100.0f;
	sum->z = (sum->z - brd->pos.z) / 100.0f;
	return sum;
}

Vector3d * birds_matchSpeed(Bird * brd) {
	Vector3d * sum = vector3d_ini(0.0f, 0.0f, 0.0f);
	int i;
	for(i = 0; i < birdsList_size; i++) {
		if(brd->id != birdsList[i].id) {
			sum->x += birdsList[i].vel.x / (double) (birdsList_size-1);
			sum->y += birdsList[i].vel.y / (double) (birdsList_size-1);
			sum->z += birdsList[i].vel.z / (double) (birdsList_size-1);	
		}
	}
	sum->x = (sum->x - brd->vel.x) / 8.0f;
	sum->y = (sum->y - brd->vel.y) / 8.0f;
	sum->z = (sum->z - brd->vel.z) / 8.0f;
	return sum;
}

Vector3d * birds_keepDistance(Bird * brd) {
	Vector3d * sum = vector3d_ini(0.0f, 0.0f, 0.0f);		
	Vector3d * aux = NULL;
	int i;
	for(i = 0; i < birdsList_size; i++) {
		if(brd->id != birdsList[i].id) {
			aux = vector3d_sub(NULL, brd->pos, birdsList[i].pos);
			if(vector3d_abs(*aux) < 10.0f) {
				free(aux);
				aux = vector3d_sub(NULL, brd->pos, birdsList[i].pos);
				vector3d_sub(sum, *sum, *aux);
			}
			free(aux);
		}
	}
	return sum;
}

void birds_limitSpeed(Bird * brd) {
	if(brd->vel.x > 50) brd->vel.x = 50;
	if(brd->vel.x <-50) brd->vel.x =-50;
	if(brd->vel.y > 50) brd->vel.y = 50;
	if(brd->vel.y <-50) brd->vel.y =-50;
	if(brd->vel.z > 50) brd->vel.z = 50;
	if(brd->vel.z <-50) brd->vel.z =-50;
}

void birds_limitPosition(Bird * brd) {
	if(brd->pos.x <   0) brd->pos.x = 0;
	if(brd->pos.x > 999) brd->pos.x = 999;
	if(brd->pos.y <   0) brd->pos.y = 0;
	if(brd->pos.y > 999) brd->pos.y = 999;
	if(brd->pos.z <   0) brd->pos.z = 0;
	if(brd->pos.z > 999) brd->pos.z = 999;
}

void birds_fly(Bird * brd, Bird * brdTar) {
	Vector3d * v1 = birds_flyCenter(brd);
	Vector3d * v2 = birds_keepDistance(brd);
	Vector3d * v3 = birds_matchSpeed(brd);

	vector3d_add(&brdTar->vel, brd->vel, *v1, *v2, *v3);
	free(v1);
	free(v2);
	free(v3);
	birds_limitSpeed(brdTar);

	brdTar->pos.x += brdTar->vel.x;
	brdTar->pos.y += brdTar->vel.y;
	brdTar->pos.z += brdTar->vel.z;
	birds_limitPosition(brdTar);
}

void birds_clone(Bird * tar, Bird * src) {
	int i;
	for(i = 0; i < birdsList_size; i++) 
		tar[i] = src[i];
}

void birds_gather(int size) {
	int i;
	Bird * gBuff = (Bird *) malloc(sizeof(Bird) * (par.N * size));
	for(i = 0; i < (par.N * size); i++)
		gBuff[i].id = -1;

	MPI_Allgather(birdsListNext, par.N, MPI_Bird, 
		gBuff, par.N, MPI_Bird, MPI_COMM_WORLD);

	for(i = 0; i < (par.N * size); i++)
		if(gBuff[i].id != -1) {
			birdsListNext[gBuff[i].id] = gBuff[i];
		}

	free(gBuff);
}


Vector3d * vector3d_ini(double x, double y, double z) {
	Vector3d * res = (Vector3d *) malloc(sizeof(Vector3d));
	res->x = x;
	res->y = y;
	res->z = z;
	return res;
}

void vector3d_show(Vector3d v, const char * ch) {
	printf("(%.1f, %.1f, %.1f) = %.1f%s", v.x, v.y, v.z, vector3d_abs(v), ch);
}

double vector3d_abs(Vector3d v) {
	return sqrt(v.x*v.x  +  v.y*v.y  +  v.z*v.z);
}

Vector3d * vector3d_sub(Vector3d * res, Vector3d a, Vector3d b) {
	res = (res == NULL) ? (Vector3d *) malloc(sizeof(Vector3d)) : res;
	res->x = a.x - b.x;
	res->y = a.y - b.y;
	res->z = a.z - b.z;
	return res;
}

Vector3d * vector3d_add(Vector3d * res, Vector3d a, Vector3d b, Vector3d c, Vector3d d) {
	res = (res == NULL) ? (Vector3d *) malloc(sizeof(Vector3d)) : res;
	res->x = a.x + b.x + c.x + d.x;
	res->y = a.y + b.y + c.y + d.y;
	res->z = a.z + b.z + c.z + d.z;
	return res;	
}

void vector3d_loadFree(Vector3d * tar, Vector3d * src) {
	tar->x = src->x;
	tar->y = src->y;
	tar->z = src->z;
	free(src);
}