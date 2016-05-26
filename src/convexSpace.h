// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convexSpace module manages affine spaces

#ifndef CONVEX_SPACE_H
#define CONVEX_SPACE_H

#include <GL/freeglut.h>
#include <stdbool.h>

// Represents affine space
struct convexSpace {
	int dim;           // dimension
	int coordsCnt;     // number of coordinates
	GLdouble *pos;      // a point in the space, it should be inside the figure when assigned
	GLdouble *ortBasis; // orthonormal basis
	GLdouble *normal, normalPos; // normal (in another space) and position within it (dist from origin)
};

struct convexFig;
struct convexFigList;


// Creates 0-dimensional space (reuses given space or allocates it)
extern void convexSpaceCreateVert(struct convexSpace **pSpace, GLdouble *pos);

// Creates space generated by fig (reuses given space or allocates it)
extern void convexSpaceCreate(struct convexSpace **pSpace, struct convexFig *fig);

// Creates copy of src (reuses given dst space or allocates it)
extern void convexSpaceCopy(struct convexSpace *src, struct convexSpace **pDst);

// Deletes given space (and assigns zero)
extern void convexSpaceDestroy(struct convexSpace **pSpace);


// Expands given space by given vertex
extern void convexSpaceExpand(struct convexSpace *space, struct convexSpace *vert);

// Expands given space by given vertex canceling previous expansion
extern void convexSpaceReexpand(struct convexSpace *space, struct convexSpace *vert);

// Decreases dimension of space by one removing the last basis vector and setting it as normal
extern void convexSpaceDecreaseDim(struct convexSpace *space);


// Assigns space to the fig (copies it to fig->space, adds fig to convexSpaces)
extern void convexSpaceAssign(struct convexSpace *space, struct convexFig *fig);

// Unassigns space from fig
extern void convexSpaceUnassign(struct convexFig *fig);


// Sets space->pos to the center of gravity of given vertices
extern void convexSpaceCenterPos(struct convexSpace *space, struct convexFigList *vertices);

// Moves space to cross the given vertex, normalPos is updated
extern void convexSpaceMoveTo(struct convexSpace *space, struct convexFig *vertex);

// Calculates normals (and normalPos) of the space in another space
extern void convexSpaceNormalCalc(struct convexSpace *space, struct convexSpace *inSpace);


// Returns whether given spaces are equal
extern bool convexSpaceEq(struct convexSpace *space1, struct convexSpace *space2);

// Returns whether space1 contains (or is equal to) space2
extern bool convexSpaceContains(struct convexSpace *space1, struct convexSpace *space2);

// Returns distance of space and given vertex (resp. vert->pos)
extern GLdouble convexSpaceDist(struct convexSpace *space, struct convexSpace *vert);

// Returns squared distance of space and given vertex
extern GLdouble convexSpaceDistSq(struct convexSpace *space, struct convexSpace *vert);

// Returns oriented distance of space and given vertex (in direction of space->normal)
extern GLdouble convexSpaceOrientedDist(struct convexSpace *space, struct convexSpace *vert);


// Compares vert#->pos lexicographically
extern int convexSpaceCmpLex(struct convexSpace *vert1, struct convexSpace *vert2);

#endif
