// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convexHullUtil provides useful functions for convexHull module

#ifndef CONVEX_HULL_UTIL_H
#define CONVEX_HULL_UTIL_H

#include <stdbool.h>
struct convexFig;
struct convexFigList;

// Creates figure newFig connecting fig and vert (not in the space of fig)
extern bool convexHullUtilExpandDim(struct convexFig *fig, struct convexFig *vert, struct convexFig **newFig, struct convexFigList **inconsistent);

// Expands fig with the given vertices in the space generated by fig, returns true if expansion was needed
extern bool convexHullUtilExpand(struct convexFig *fig, struct convexFigList *vertices, struct convexFigList **inconsistent);

// Repairs boundary of the figs in list (and the figs it breaks), returns inconsistent figs of higher dimension
extern void convexHullUtilRepair(struct convexFigList **pList, struct convexFigList **inconsistent);

// Removes faces of fig which have outer vertices in vertices list, returns its count and inconsistent neighbours of fig
extern int convexHullUtilWrongFacesRm(struct convexFig *fig, struct convexFigList *vertices, struct convexFigList **inconsistent);

// Completes fig boundary, which has to be the convex hull of given vertices
extern void convexHullUtilComplete(struct convexFig *fig, struct convexFigList **vertices, struct convexFigList **inconsistent);

// Create initial simplex face of given dimension of the convex hull of the vertices
extern struct convexFig *convexHullUtilInitialFace(struct convexFigList *vertices, int dim, struct convexFigList **inconsistent);

// Creates convex hull of the given vertices
extern struct convexFig *convexHullUtilCreate(struct convexFigList *vertices, struct convexFigList **inconsistent);

// Calculates normals of the (highest-dimension) faces of fig
extern void convexHullUtilNormalsCalc(struct convexFig *fig);

// Adds fig to the list and marks it inconsistent
extern void convexHullUtilAddInconsistent(struct convexFigList **list, struct convexFig *fig);

#endif
