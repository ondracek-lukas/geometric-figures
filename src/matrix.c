// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// column-major order
// matrix[row + col*ROWS]

#include "matrix.h"

#include <GL/freeglut.h>
#include <math.h>

#include "safe.h"

void matrixZero(GLfloat *matrix, int areaSize) {
	int i;
	for (i=0; i<areaSize; i++)
		matrix[i]=0;
}

void matrixIdentity(GLfloat *matrix, int size) {
	int i, j;
	for (i=0; i<size; i++)
		for (j=0; j<size; j++)
			if (i==j)
				matrix[i+j*size]=1;
			else
				matrix[i+j*size]=0;
}

void matrixProduct(GLfloat *a, GLfloat *b, GLfloat *c, int s1, int s2, int s3) {
	int i, j, k;
	for (i=0; i<s1; i++)
		for (k=0; k<s3; k++) {
			c[i+k*s1]=0;
			for (j=0; j<s2; j++)
				c[i+k*s1]+=a[i+j*s1]*b[j+k*s2];
		}
}

void matrixCopy(GLfloat *a, GLfloat *b, int areaSize) {
	int i;
	for (i=0; i<areaSize; i++)
		b[i]=a[i];
}

void matrixScale(GLfloat *matrix, GLfloat scalar, int areaSize) {
	int i;
	for (i=0; i<areaSize; i++)
		matrix[i]*=scalar;
}

GLfloat matrixScalarProduct(GLfloat *a, GLfloat *b, int areaSize) {
	int i;
	GLfloat sum=0;
	for (i=0; i<areaSize; i++)
		sum+=a[i]*b[i];
	return sum;
}

GLfloat matrixVectorNormSq(GLfloat *vector, int size) {
	return matrixScalarProduct(vector, vector, size);
}

GLfloat matrixVectorNorm(GLfloat *vector, int size) {
	return sqrt(matrixVectorNormSq(vector, size));
}

void matrixVectorNormalize(GLfloat *vector, int size) {
	matrixScale(vector, 1/matrixVectorNorm(vector, size), size);
}

void matrixAdd(GLfloat *matrix, GLfloat *matrix2, int areaSize) {
	int i;
	for (i=0; i<areaSize; i++)
		matrix[i]+=matrix2[i];
}

void matrixAddScaled(GLfloat *matrix, GLfloat multiplier, GLfloat *matrix2, int areaSize) {
	int i;
	for (i=0; i<areaSize; i++)
		matrix[i]+=multiplier*matrix2[i];
}

// basis contains basisDim vectors of spaceDim size in columns - basis[vectorElem + vectorIndex*VECTORS]
void matrixOrtVectorToBasis(GLfloat *basis, GLfloat *vector, int basisDim, int spaceDim) {
	// basis have to be orthonormal
	int i;
	for (i=0; i<basisDim; i++)
		matrixAddScaled(vector, -matrixScalarProduct(vector, basis+i*spaceDim, spaceDim), basis+i*spaceDim, spaceDim);
}


GLfloat *matrixRotation(int size, int axis1, int axis2, GLfloat angle) {
	// generates temporary matrix, valid till next call
	static GLfloat *matrix=0;
	static int lsize=0, laxis1, laxis2;
	if (lsize!=size) {
		free(matrix);
		matrix=safeMalloc(size*size*sizeof(GLfloat));
		lsize=size;
		laxis1=axis1;
		laxis2=axis2;
		matrixIdentity(matrix, size);
	} else if ((axis1!=laxis1) || (axis2!=laxis2)) {
		matrix[laxis1+laxis1*size]=matrix[laxis2+laxis2*size]=1;
		matrix[laxis1+laxis2*size]=matrix[laxis2+laxis1*size]=0;
		laxis1=axis1;
		laxis2=axis2;
	}
	matrix[axis1+axis1*size]=  matrix[axis2+axis2*size]=cos(angle*M_PI/180);
	matrix[axis1+axis2*size]=-(matrix[axis2+axis1*size]=sin(angle*M_PI/180));
	return matrix;
}

void matrixPerspective(int srcdim, int dstdim, GLfloat *campos, GLfloat *vector) {
	int i;
	GLfloat a1=1, a2=1, b=1;
	for (i=srcdim-1; i>=dstdim; i--) {
		a1*=sqrt(campos[i]*campos[i]-1); // distance from cam without scaling distances
		a2*=campos[i];                   // distance from cam without scaling sizes of vertices
		b*=campos[i]-vector[i];
	}
	a1/=b;
	a2/=b;
	for (i=0; i<dstdim; i++)
		vector[i]*=a1;
	vector[i]=a2;
}
