// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convex and its submodules convex* generate and maintain convex hull of the figure

#ifndef CONVEX_H
#define CONVEX_H

#include <GL/freeglut.h>
#include <stdbool.h>

// Determines whether convex is attached to and synchronized with figure, read-only
extern struct figureData *convexAttached;

// Attaches convex to given figure
// Returns false if figure was inconsistent, repairing it
extern bool convexAttach(struct figureData *figure);

// Detaches convex from figure
extern void convexDetach();

// When attached, changes in figure module has to be reflected here:

	// Adds new vertex, which was already added in figure module
	extern void convexVertexAdd(int index);

	// Removes vertex, which is going to be removed from figure module
	extern void convexVertexRm(int index);

	// Moves vertex, which is going to be moved in figure module
	extern void convexVertexMove(int index, GLfloat *pos);


// convexHull.c:

	// Determines whether to recalculate convex hull when needed (or break faces only)
	// Can be changed anytime
	extern bool convexHull;


// Updates convex hull (if convexHull)
extern void convexUpdateHull();

// Destroys all faces (only vertices remains)
extern void convexDestroyHull();

// Updates convex hull of the given figure
// No figure is attached after call
// Returns false on loop
extern bool convexUpdateHullAtOnce(struct figureData *figure);


// convexInteract.c:

	// Determines whether convexInteract is active, read-only
	extern bool convexInteract;

	// Key press event on convexInteract module
	extern void convexInteractKeyPress();


// Internal convex* data structures
extern struct convexFigList *convexFigure;
extern struct convexFigList *convexFreeVertices;
extern struct convexFig ***convexShadow;

#endif
