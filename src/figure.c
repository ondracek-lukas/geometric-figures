// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "figure.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <math.h>

#include "convex.h"
#include "safe.h"
#include "matrix.h"
#include "console.h"

struct figureData figureData;

GLfloat *figureRotMatrix;
GLfloat figureScale=1;

static bool boundaryChanged=1;

static void setDim(int dim);
static int checkTopology();
static void destroyFigure();
static void updateScale();

void figureInit() {
	figureData.dim=-1;
	figureData.count=0;
	figureData.vertices=0;
	figureData.boundary=0;
}

static void setDim(int dim) {
	figureRotMatrix=safeMalloc(dim*dim*sizeof(GLfloat));
	matrixIdentity(figureRotMatrix, dim);
	figureData.count=safeCalloc(dim+1, sizeof(GLint));
	figureData.boundary=safeCalloc(dim+1, sizeof(GLfloat **));
	figureData.dim=dim;
}

void figureNew(int dim) {
	destroyFigure();
	if (dim>=0)
		setDim(dim);
	if (convexHull)
		convexAttach();
}

int figureSave(char *path) {
	FILE *f;
	int dim, i;
	GLfloat pos[figureData.dim];
	if (figureData.dim<0)
		return 0;
	f=fopen(path, "wb");
	if (f==0)
		return 0;
	fwrite(&figureData.dim, sizeof(GLint), 1, f);
	fwrite(figureData.count, sizeof(GLint), 1, f);
	for (i=0; i<figureData.count[0]; i++) {
		matrixProduct(figureRotMatrix, figureData.vertices[i], pos, figureData.dim, figureData.dim, 1);
		fwrite(pos, sizeof(GLfloat), figureData.dim, f);
	}
	for (dim=1; dim<=figureData.dim; dim++) {
		fwrite(figureData.count+dim, sizeof(GLint), 1, f);
		for (i=0; i<figureData.count[dim]; i++)
			fwrite(figureData.boundary[dim][i], sizeof(GLint), figureData.boundary[dim][i][0]+1, f);
	}
	fclose(f);
	return 1;
}

int figureOpen(char *path) {
	FILE *f;
	GLint i, j, k;
	f=fopen(path, "rb");
	if (f==0)
		return 0;
	destroyFigure();
	fread(&i, sizeof(GLint), 1, f);
	if ((i<0) || (i>safeMaxDim)) {
		fclose(f);
		return 0;
	}
	setDim(i);
	fread(figureData.count, sizeof(GLint), 1, f);
	if (figureData.count[0]<0) {
		fclose(f);
		destroyFigure();
		return 0;
	}
	figureData.vertices=safeCalloc(figureData.count[0], sizeof(GLfloat *));
	for (i=0; i<figureData.count[0]; i++) {
		figureData.vertices[i]=safeMalloc(figureData.dim*sizeof(GLfloat));
		fread(figureData.vertices[i], sizeof(GLfloat), figureData.dim, f);
		if (!safeCheckPos(figureData.vertices[i], figureData.dim)) {
			fclose(f);
			destroyFigure();
			return 0;
		}
	}
	for (i=1; i<=figureData.dim; i++) {
		fread(figureData.count+i, sizeof(GLint), 1, f);
		if (figureData.count[i]<0) {
			fclose(f);
			destroyFigure();
			return 0;
		}
		figureData.boundary[i]=safeCalloc(figureData.count[i], sizeof(GLint *));
		for (j=0; j<figureData.count[i]; j++) {
			fread(&k, sizeof(GLint), 1, f);
			if (k<0) {
				fclose(f);
				destroyFigure();
				return 0;
			}
			figureData.boundary[i][j]=safeMalloc((k+1)*sizeof(GLint));
			figureData.boundary[i][j][0]=k;
			fread(figureData.boundary[i][j]+1, sizeof(GLint), k, f);
		}
	}
	fclose(f);
	if (!checkTopology()) {
		figureResetBoundary();
		consolePrintErr("Error: Broken topology, boundary was removed");
	}
	updateScale();

	boundaryChanged=1;
	if (convexHull)
		convexAttach();
	return 1;
}

static int checkTopology() {
	int dim, i, j;
	for (dim=1; dim<=figureData.dim; dim++)
		for (i=0; i<figureData.count[dim]; i++)
			for (j=1; j<=figureData.boundary[dim][i][0]; j++)
				if ((figureData.boundary[dim][i][j]<0) || (figureData.boundary[dim][i][j]>=figureData.count[dim-1]))
					return 0;
	return 1;
}

void figureBoundaryChanged() {
	boundaryChanged=1;
}

void figureResetBoundary() {
	int i, j;
	boundaryChanged=1;
	if (convexAttached) {
		convexDestroyHull();
	} else {
		for (i=1; i<=figureData.dim; i++) {
			for (j=0; j<figureData.count[i]; j++)
				free(figureData.boundary[i][j]);
			free(figureData.boundary[i]);
			figureData.boundary[i]=0;
			figureData.count[i]=0;
		}
	}
	if (convexHull) {
		if (!convexAttached)
			convexAttach();
		convexUpdateHull();
	}
}

void figureResetRotation() {
	matrixIdentity(figureRotMatrix, figureData.dim);
}

static void destroyFigure() {
	int i, j;
	convexDetach();
	if (figureData.dim<0)
		return;
	boundaryChanged=1;
	for (i=0; i<figureData.count[0]; i++)
		free(figureData.vertices[i]);
	free(figureData.vertices);
	figureData.vertices=0;
	for (i=1; i<=figureData.dim; i++) {
		for (j=0; j<figureData.count[i]; j++)
			free(figureData.boundary[i][j]);
		free(figureData.boundary[i]);
	}
	free(figureData.boundary);
	figureData.boundary=0;
	free(figureData.count);
	figureData.count=0;
	free(figureRotMatrix);
	figureData.dim=-1;
	
}


void figureRotate(int axis1, int axis2, GLfloat angle) {
	static GLfloat *matrix=0;
	static int lsize=0;
	GLfloat *matrix2;
	if (lsize!=figureData.dim) {
		free(matrix);
		lsize=figureData.dim;
		matrix=safeMalloc(lsize*lsize*sizeof(GLfloat));
	}
	matrixProduct(matrixRotation(lsize, axis1, axis2, angle), figureRotMatrix, matrix, lsize, lsize, lsize);
	matrix2=figureRotMatrix;
	figureRotMatrix=matrix;
	matrix=matrix2;

}

static void updateScale() {
	GLint i, j;
	GLfloat sum;
	GLfloat farest=1;
	for (i=0; i<figureData.count[0]; i++) {
		sum=0;
		for (j=0; j<figureData.dim; j++)
			sum+=figureData.vertices[i][j]*figureData.vertices[i][j];
		sum=sqrt(sum);
		if (farest<sum)
			farest=sum;
	}
	figureScale=1/farest;
}

int figureVerticesOfFaces(int ***facevertOut) {
	int (*neighbour)[2]=safeMalloc(2*(figureData.count[0])*sizeof(int));
	static int **facevert=0;
	static int faces=0, validFaces=0;
	int i, j, e, v1, v2;
	int broken;
	if (!boundaryChanged) {
		*facevertOut=facevert;
		return validFaces;
	}
	for (i=0; i<faces; i++)
		free(facevert[i]);
	free(facevert);
	faces=figureData.count[2];
	facevert=safeCalloc(faces, sizeof(int *));
	for (i=0, validFaces=0; i<faces; i++) {
		broken=0;
		free(facevert[validFaces]);
		for (j=0; j<figureData.count[0]; j++) {
			neighbour[j][0]=-1;
			neighbour[j][1]=-1;
		}
		facevert[validFaces]=safeMalloc((figureData.boundary[2][i][0]+1)*sizeof(int));
		facevert[validFaces][0]=figureData.boundary[2][i][0];
		for (j=1; j<=facevert[validFaces][0]; j++) {
			e=figureData.boundary[2][i][j];
			v1=figureData.boundary[1][e][1];
			v2=figureData.boundary[1][e][2];
			if (neighbour[v1][0]==-1)
				neighbour[v1][0]=v2;
			else if (neighbour[v1][1]==-1)
				neighbour[v1][1]=v2;
			else {
				consolePrintErr("Inconsistent border of 2D faces, skipping drawing");
				broken=1;break;
			}
			if (neighbour[v2][0]==-1)
				neighbour[v2][0]=v1;
			else if (neighbour[v2][1]==-1)
				neighbour[v2][1]=v1;
			else {
				consolePrintErr("Inconsistent border of 2D faces, skipping drawing");
				broken=1;break;
			}
		}
		if (broken)
			continue;
		for (j=0; j<facevert[validFaces][0]; j++) {
			if (v1<0) {
				consolePrintErr("Inconsistent border of 2D faces, skipping drawing");
				broken=1;break;
			}
			facevert[validFaces][j+1]=v1;
			if (neighbour[v1][0]!=v2) {
				v2=v1;
				v1=neighbour[v1][0];
			} else {
				v2=v1;
				v1=neighbour[v1][1];
			}
			neighbour[v2][0]=-1;
			neighbour[v2][1]=-1;
		}
		if (broken)
			continue;
		validFaces++;
	}
	free(neighbour);
	*facevertOut=facevert;
	boundaryChanged=0;
	return validFaces;
}

void figureVertexMove(int vertex, GLfloat *pos) {
	if (!convexAttached)
		convexAttach();
	convexVertexMove(vertex, pos);
	matrixCopy(pos, figureData.vertices[vertex], figureData.dim);
	updateScale();
}

int figureVertexAdd(GLfloat *pos) {
	if (!convexAttached)
		convexAttach();
	figureData.vertices=safeRealloc(figureData.vertices, ++figureData.count[0]*sizeof(GLfloat *));
	GLfloat *pos2=safeMalloc(figureData.dim*sizeof(GLfloat));
	matrixCopy(pos, pos2, figureData.dim);
	figureData.vertices[figureData.count[0]-1]=pos2;
	updateScale();
	convexVertexAdd(figureData.count[0]-1);
	return figureData.count[0]-1;
}

void figureVertexRm(int vertex) {
	int i;
	if (!convexAttached)
		convexAttach();
	convexVertexRm(vertex);
	free(figureData.vertices[vertex]);
	for (i=vertex; i<figureData.count[0]-1; i++)
		figureData.vertices[i]=figureData.vertices[i+1];
	figureData.count[0]--;
	figureData.vertices=safeRealloc(figureData.vertices, figureData.count[0]*sizeof(GLfloat *));
	updateScale();
}


int figureDistCmpZero(GLfloat distance, GLfloat tolerance) {
	return (fabs(distance)*figureScale>tolerance?(distance>0?1:-1):0);
}

int figureDistCmpZeroSq(GLfloat squaredDistance, GLfloat tolerance) {
	return squaredDistance*figureScale*figureScale>tolerance*tolerance;
}

int figureDistCmp(GLfloat a, GLfloat b, GLfloat tolerance) {
	return figureDistCmpZero(a-b, tolerance);
}
