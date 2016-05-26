// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// matrix provides different matrix operations

// column-major order
// matrix[row + col*ROWS]

#ifndef MATRIX_H
#define MATRIX_H

#include <GL/freeglut.h>

// Sets areaSize items to zero
extern void matrixZero(GLdouble *matrix, int areaSize);
extern void matrixZeroF(GLfloat *matrix, int areaSize);

// Changes matrix (size x size) to identity matrix
extern void matrixIdentity(GLdouble *matrix, int size);

// Computes product of matrices a (s1 x s2) and b (s2 x s3) into c (s1 x s3)
extern void matrixProduct(GLdouble *a, GLdouble *b, GLdouble *c, int s1, int s2, int s3);

// Copies areaSize items from a to b
extern void matrixCopy(GLdouble *a, GLdouble *b, int areaSize);
extern void matrixCopyF(GLfloat *a, GLfloat *b, int areaSize);

// Multiplies areaSize items by scalar
extern void matrixScale(GLdouble *matrix, GLdouble scalar, int areaSize);
extern void matrixScaleF(GLfloat *matrix, GLfloat scalar, int areaSize);

// Returns dot product of a and b
extern GLdouble matrixScalarProduct(GLdouble *a, GLdouble *b, int areaSize);

// Returns squared norm of the vector
extern GLdouble matrixVectorNormSq(GLdouble *vector, int size);

// Returns norm of the vector
extern GLdouble matrixVectorNorm(GLdouble *vector, int size);

// Normalizes given vector
extern void matrixVectorNormalize(GLdouble *vector, int size);

// Adds matrix2 to matrix
extern void matrixAdd(GLdouble *matrix, GLdouble *matrix2, int areaSize);

// Adds matrix2 multiplied by multiplier to matrix
extern void matrixAddScaled(GLdouble *matrix, GLdouble multiplier, GLdouble *matrix2, int areaSize);
extern void matrixAddScaledF(GLfloat *matrix, GLfloat multiplier, GLfloat *matrix2, int areaSize);

// Orthogonalizes vector to given basis
// Basis contains basisDim vectors of spaceDim size in columns - basis[vectorElem + vectorIndex*VECTORS]
// Basis have to be orthonormal
extern void matrixOrtVectorToBasis(GLdouble *basis, GLdouble *vector, int basisDim, int spaceDim);

// Creates read-only temporary (valid till next call) matrix (size x size) of rotation
extern GLdouble *matrixRotation(int size, int axis1, int axis2, GLdouble angle);

// Iterative perspective projection of the vector (of length srcdim) according to the given camera position
// campos[0] and campos[1] assumed to be 0
// First dstdim items of vector will be changed to output vector
// vector[dstdim] will be scaling factor at given position
extern void matrixPerspective(int srcdim, int dstdim, GLdouble *campos, GLdouble *vector);

#endif
