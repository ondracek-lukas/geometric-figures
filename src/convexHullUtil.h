// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convexHullUtil provides useful functions for convexHull module

#ifndef CONVEX_HULL_UTIL_H
#define CONVEX_HULL_UTIL_H

#include <stdbool.h>
struct convexFig;
struct convexFigList;
struct convexSpace;

// Creates new figure by expanding fig using newVertices into higher dimension
// if fig is of dimension d, newSpace should be of dimension d+1 containing fig and newVertices
// fig+newVertices should generate newSpace
extern struct convexFig *convexHullUtilExpandDim(struct convexFig *fig, struct convexFigList *newVertices, struct convexSpace *newSpace);

// Completes fig boundary, which has to be the convex hull of given vertices
extern void convexHullUtilComplete(struct convexFig *fig, struct convexFigList *vertices);

// Creates convex hull of the given vertices
extern struct convexFig *convexHullUtilCreate(struct convexFigList *vertices);

// Creates new list which contains just the vertices/figures from the original one lying in the space
extern struct convexFigList *convexHullUtilSpaceFilter(struct convexFigList *figures, struct convexSpace *inSpace);

// Calculates normals of the (highest-dimension) faces of fig
extern void convexHullUtilNormalsCalc(struct convexFig *fig);

#endif
