// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// This file is intended to be included only in convexFig.h
// convexFigMark allows marking figs

#include <stdbool.h>
struct convexFig;

// List of all marks independent to each other
enum convexFigMarkId {
	convexFigMarkIdTrue=-1, // always true, cannot be changed
	convexFigMarkIdLayer=0,
	convexFigMarkIdHash,
	convexFigMarkIdHullInconsistent,
	convexFigMarkIdHull
};
#define convexFigMarkCount 4

// Unmarks all (in constant time)
extern void convexFigMarkReset(enum convexFigMarkId mark);

// Marks fig
extern void convexFigMarkSet(struct convexFig *fig, enum convexFigMarkId mark);

// Unmarks fig
extern void convexFigMarkClear(struct convexFig *fig, enum convexFigMarkId mark);

// Determines whether the fig is marked
extern bool convexFigMarkGet(struct convexFig *fig, enum convexFigMarkId mark);
