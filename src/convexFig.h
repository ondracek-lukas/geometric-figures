// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convexFig provides another data structure for storing opened figure (in addition to figure module),
// allowing easier modifications

#ifndef CONVEX_FIG_H
#define CONVEX_FIG_H

struct convexSpace;

// Other parts of convexFig, read for info, don't include directly
#include "convexFigMark.h"
#include "convexFigList.h"
#include "convexFigBst.h"

// Stores figure in convex module
struct convexFig {
	int index;
	unsigned int hash;
	unsigned int mark[convexFigMarkCount];
	struct convexFigList *parents;
	struct convexFigList *boundary; // childs
	struct convexSpace *space;
};

// Creates new convexFig
extern struct convexFig *convexFigNew();

// Deletes convexFig (it has to be detached from all parents and childs)
extern void convexFigDelete(struct convexFig *fig);

// Attaches (or detaches) child to parent as the part of its boundary
extern void convexFigBoundaryAttach(struct convexFig *parent, struct convexFig *child);
extern int convexFigBoundaryDetach(struct convexFig *parent, struct convexFig *child);

// Detaches convexFig from parents and childs, delete convexFig and all new orphans
extern void convexFigDestroy(struct convexFig *fig);


// Touches fig and all its ancestors, touched figs needs to be updated when exporting to figure module
extern void convexFigTouch(struct convexFig *fig);

// Gets ancestors/successors of given dimension marked by filterMark
// skipMarked figures are skipped, the others are marked; not-skipping if mark-true
extern int convexFigGetLayer(struct convexFig *fig, int dim, enum convexFigMarkId filterMark, enum convexFigMarkId skipMark, struct convexFigList **layer);

// Calculates hash of fig except its hash-marked components and combines it with given hash
// Intended use: reset hash-mark, assign 0 to hash, call convexFigHashCalc with all selected figs
// All vertices are hash-marked
extern int convexFigHashCalc(struct convexFig *fig, unsigned int *hash);

// Returns figure from list with given hash and all vertices (given count) hash-marked or 0
extern struct convexFig *convexFigHashFind(struct convexFigList *list, unsigned int hash, int vertCount);

#endif
