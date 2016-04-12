// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// figure module manages opened geometric figure

#ifndef FIGURE_H
#define FIGURE_H

#include <GL/freeglut.h>
#include <stdbool.h>


// Represents figure
struct figureData {
	GLint dim;          // dimension of the figure (number of coordinates)
	GLint *count;       // counts of faces of given dimension (0-vertices, 1-edges, ...)
	GLdouble **vertices; // [vertex index][axis] = coordinate
	GLint ***boundary;  // [face dimension][face index] = face border (array of (dim-1)-faces)
	                    // [face dimension][face index][bordering face index] = index in boundary[dim-1]
	// vertex index         from 0 to count[0]-1
	// face dimension       from 1 to dim  (0 represented by vertices)
	// face index           from 0 to count[face dimension]-1
	// bordering face index from 1 to boundary[face dimension][face index][0]
};

// Currently opened figure, read-only except boundary (call figureBoundaryChanged() after change)
extern struct figureData figureData;

// Current figure rotation, read-only
extern GLdouble *figureRotMatrix;

// Current figure scale, read-only
extern GLdouble figureScale;


// Module initialization, to be called only once
extern void figureInit();


// Resets figure to its initial rotation
// Throws exception using script module
extern void figureResetRotation(); // [SCRIPT_NAME: resetRotation]

// Rotates figure in the plane of given axes by given angle
extern void figureRotate(int axis1, int axis2, GLdouble angle);


// Figure modifications:

	// Creates new (empty) figure of given dimension
	extern void figureNew(int dim);

	// Opens given figure, the figure parameter is freed
	// Throws exception using script module (returning false)
	extern bool figureOpen(struct figureData *figure, bool preserveRotation);

	// Destroys given figure, hard frees it
	extern void figureDestroy(struct figureData *figure, bool hard);

	// Moves vertex with given index to given position
	extern void figureVertexMove(int vertex, GLdouble *pos);

	// Adds new vertex at given position
	extern int figureVertexAdd(GLdouble *pos);

	// Remove vertex with given index
	extern void figureVertexRm(int vertex);

	// Removes boundary and (if convexHull) recalculates it again
	// Throws exception using script module
	extern void figureResetBoundary(); // [SCRIPT_NAME: resetBoundary]

	// Call on figureData.boundary change
	extern void figureBoundaryChanged();


// Gets array of faces boundaries as array of vertices indices (one * for out param) read-only, returns count
extern int figureVerticesOfFaces(int ***facevertOut);


// Approximately compares distances
extern int figureDistCmpZero(GLdouble distance);
extern int figureDistCmpZeroSq(GLdouble squaredDistance);
extern int figureDistCmp(GLdouble a, GLdouble b);

#endif
