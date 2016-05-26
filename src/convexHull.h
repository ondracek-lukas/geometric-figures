// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convexHull creates and maintains convex hull

#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

struct convexFig;
#include <stdbool.h>

// declared also in convex.h:
	// Determines whether to recalculate convex hull when needed (or only brake faces)
	// Can be changed anytime
	extern bool convexHull;

// Adds new vertex
extern void convexHullVertAdd(struct convexFig *vert);

// Removes specified vertex
extern void convexHullVertRm(struct convexFig *vert);

// Creates convex hull
extern void convexHullCreate();

// Updates convex hull
extern void convexHullUpdate();

// Break faces (of all dimensions) incident with the vertex
extern void convexHullBreakNearVert(struct convexFig *vertIn);

// Destroy hull (only vertices remains)
extern void convexHullDestroy();

#endif
