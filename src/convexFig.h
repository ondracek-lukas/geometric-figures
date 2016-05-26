// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convexFig provides another data structure for storing opened figure (in addition to figure module),
// allowing easier modifications

#ifndef CONVEX_FIG_H
#define CONVEX_FIG_H

struct convexSpace;

// Other parts of convexFig, read for info, don't include directly
#include "convexFigMark.h"
#include "convexFigList.h"
#include "convexFigHash.h"

// Stores figure in convex module
struct convexFig {
	int index;
	unsigned int hash;
	unsigned int mark[convexFigMarkCount];
	struct convexFigList *parents;
	struct convexFigList *boundary; // children
	struct convexFigList *vertices; // vertices in boundary
	struct convexSpace *space;
};

// Total number of existing figures (read-only)
extern int convexFigCount;

// Creates new convexFig
extern struct convexFig *convexFigNew();

// Deletes convexFig
// it has to be detached from all parents and childs and removed from hash
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

#endif
