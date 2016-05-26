// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// This file is intended to be included only in convexFig.h
// convexFigMark allows marking figs

#include <stdbool.h>
struct convexFig;
struct convexFigList;

// List of all marks independent to each other
enum convexFigMarkId {
	convexFigMarkIdTrue=-1, // always true, cannot be changed
		// can be used only with convexFigMarkSet and convexFigMarkGet
	convexFigMarkIdLayer=0,
	convexFigMarkIdHash,
	convexFigMarkIdHull,
	convexFigMarkIdHullProcessed,
	convexFigMarkIdHullOneParent,
	convexFigMarkCount
};

// Unmarks all (in constant time), returns new version of marks (to be used)
extern unsigned int convexFigMarkReset(enum convexFigMarkId mark);

// Clears recursively all marks in given figures (to 0) and sets marks versions to 1
extern void convexFigMarkResetHard(struct convexFigList *allFigures);

// Marks fig using current or given version
extern void convexFigMarkSet(struct convexFig *fig, enum convexFigMarkId mark);
extern void convexFigMarkSetV(struct convexFig *fig, enum convexFigMarkId mark, unsigned int version);

// Unmarks fig (of all versions)
extern void convexFigMarkClear(struct convexFig *fig, enum convexFigMarkId mark);

// Determines whether the fig is marked (with current or given version of marks)
extern bool convexFigMarkGet(struct convexFig *fig, enum convexFigMarkId mark);
extern bool convexFigMarkGetV(struct convexFig *fig, enum convexFigMarkId mark, unsigned int version);

// Sets mark to the opposite state returning it
extern bool convexFigMarkNegGet(struct convexFig *fig, enum convexFigMarkId mark);
extern bool convexFigMarkNegGetV(struct convexFig *fig, enum convexFigMarkId mark, unsigned int version);
