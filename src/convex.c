// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convex.h"

#include "safe.h"
#include "convexFig.h"
#include "convexSpace.h"
#include "convexInteract.h"
#include "convexHull.h"
#include "debug.h"
#include "figure.h"

struct convexFigList *convexFigure=0;
struct convexFigList *convexFreeVertices=0;
struct convexFig ***convexShadow=0;
struct figureData *convexAttached=NULL;

static struct convexSpace *tmpSpace=0;
static void exportHull();

bool convexAttach(struct figureData *figure) {
	struct convexFigList *wrongDimList=0;
	struct convexFig *fig;
	int i, j, dim;
	bool inconsistent=false;
	if (convexAttached)
		convexDetach();
	if (figure->dim<0)
		return true;
	convexAttached=figure;
	convexShadow=safeMalloc((figure->dim+1)*sizeof(struct convexFig**));
	for (i=0; i<=figure->dim; i++)
		convexShadow[i]=safeMalloc(figure->count[i]*sizeof(struct convexFig*));
	for (i=0; i<figure->count[0]; i++) {
		fig=convexFigNew();
		fig->index=i;
		convexSpaceCreateVert(&tmpSpace, figure->vertices[i]);
		convexSpaceAssign(tmpSpace, fig);
		fig->hash=rand();
		convexShadow[0][i]=fig;
	}
	for (dim=1; dim<=figure->dim; dim++) {
		for (i=0; i<figure->count[dim]; i++) {
			fig=convexFigNew();
			fig->space->dim=dim;
			fig->index=i;
			convexShadow[dim][i]=fig;
			for (j=1; j<=figure->boundary[dim][i][0]; j++)
				if (convexShadow[dim-1][figure->boundary[dim][i][j]])
					convexFigBoundaryAttach(fig, convexShadow[dim-1][figure->boundary[dim][i][j]]);
			convexSpaceCreate(&tmpSpace, fig);
			convexSpaceAssign(tmpSpace, fig);
			if (fig->space->dim != dim) {
				fig->space->dim=dim;
				convexFigListAdd(&wrongDimList, fig);
				inconsistent=true;
			}
			convexFigGetLayer(fig, 0, convexFigMarkIdTrue, convexFigMarkIdTrue, &fig->vertices);
			convexFigMarkReset(convexFigMarkIdHash);
			convexFigHashCalc(fig, &fig->hash);
		}
		while (wrongDimList)
			convexFigDestroy(convexFigListRm(&wrongDimList));
	}
	for (dim=1; dim<=figure->dim; dim++) {
		for (i=0; i<figure->count[dim]; i++) {
			if (convexShadow[dim][i] && !convexShadow[dim][i]->parents) {
				convexFigListAdd(&convexFigure, convexShadow[dim][i]);
			}
		}
	}
	convexFigListDestroy(&convexFreeVertices);
	for (i=0; i<figure->count[0]; i++)
		if (!convexShadow[0][i]->parents)
			convexFigListAdd(&convexFreeVertices, convexShadow[0][i]);

	DEBUG_HULL_PROGR(debugProgrStart(figure, convexShadow);)
	convexUpdateHull();
	if (!convexHull && inconsistent && !convexInteractAborted)
		exportHull();
	return !inconsistent;
}


void convexUpdateHull() {
	if (convexHull) {
		convexInteractStart("Checking and updating convex hull...");
		//convexHullUpdate();
		convexHullCreate();
		convexInteractStop("Convex hull has been checked and updated...");
		if (convexInteractAborted) {
			convexDetach();
		} else {
			exportHull();
		}
	}
}

bool convexUpdateHullAtOnce(struct figureData *figure) {
	bool origConvexHull=convexHull;
	convexHull=false; // suppress user interaction
	convexAttach(figure);
	convexInteractAborted=false;
	convexHull=true;
	//convexHullUpdate();
	convexHullCreate();
	if (!convexInteractAborted) {
		exportHull();
	}
	convexDetach();
	convexHull=origConvexHull;
	return !convexInteractAborted;
}

void convexDestroyHull() {
	convexHullDestroy();
	exportHull();
}

static void exportHull() {
	int dim, count, i, j;
	struct convexFigList *list1, *list2=0;
	bool changed=false;
	for (dim=1; dim<=convexAttached->dim; dim++) {
		count=0;
		convexFigMarkReset(convexFigMarkIdLayer);
		for (list1=convexFigure; list1; list1=list1->next)
			count+=convexFigGetLayer(list1->fig, dim, convexFigMarkIdTrue, convexFigMarkIdLayer, &list2);
		DEBUG_HULL_VERBOSE(printf("count[%d]=%d\n", dim, count);)
		if (count!=convexAttached->count[dim]) {
			for (i=count; i<convexAttached->count[dim]; i++)
				if (convexShadow[dim][i])
					convexFigTouch(convexShadow[dim][i]);
			convexAttached->boundary[dim]=safeRealloc(convexAttached->boundary[dim], count*sizeof(GLint *));
			convexShadow[dim]=safeRealloc(convexShadow[dim], count*sizeof(struct convexFig *));
			changed=true;
		}
		for (i=0; i<count; i++) {
			if ((i<convexAttached->count[dim]) && convexShadow[dim][i]) {
				DEBUG_HULL_VERBOSE(printf("Skipping shadow[%d][%d] - fig index %d\n", dim, i, convexShadow[dim][i]->index);)
				continue;
			}
			changed=true;
			while (list2->fig->index>=0) {
				DEBUG_HULL_VERBOSE(printf("Skipping fig index %d-%d\n", dim, list2->fig->index);)
				convexFigListRm(&list2);
			}
			list2->fig->index=i;
			convexShadow[dim][i]=list2->fig;
			if (i<convexAttached->count[dim])
				free(convexAttached->boundary[dim][i]);
			list1=list2->fig->boundary;
			j=convexFigListLen(list1);
			convexAttached->boundary[dim][i]=safeMalloc((j+1)*sizeof(GLint));
			convexAttached->boundary[dim][i][0]=j;
			DEBUG_HULL_VERBOSE(printf("%d-%d-%d: %d\n", dim, i, 0, j);)
			for (j=1; list1; j++, list1=list1->next) {
				convexAttached->boundary[dim][i][j]=list1->fig->index;
				DEBUG_HULL_VERBOSE(printf("%d-%d-%d: %d\n", dim, i, j, list1->fig->index);)
			}
		}
		DEBUG_HULL_VERBOSE(
			while (list2 && (list2->fig->index>=0)) {
				// printf("Skipping fig index %d-%d\n", dim, list2->fig->index);
				convexFigListRm(&list2);
			}
			if (list2)
				safeExitErr("convex export error");)

		convexFigListDestroy(&list2);
		convexAttached->count[dim]=count;
	}
	DEBUG_HULL(
		for (dim=1; dim<=convexAttached->dim; dim++)
			for (i=0; i<convexAttached->count[dim]; i++)
				for (j=1; j<=convexAttached->boundary[dim][i][0]; j++)
					if (convexAttached->boundary[dim][i][j]<0)
						safeExitErr("convex export error");)
	if (changed) {
		figureBoundaryChanged();
	}
}

void convexDetach() {
	int dim;
	if (!convexAttached)
		return;
	convexHullDestroy();
	for (dim=0; dim<=convexAttached->dim; dim++)
		free(convexShadow[dim]);
	free(convexShadow);
	while (convexFreeVertices) {
		convexSpaceUnassign(convexFreeVertices->fig);
		convexFigDelete(convexFigListRm(&convexFreeVertices));
	}
	convexShadow=0;
	convexAttached=NULL;
}

void convexVertexAdd(int index) {
	struct convexFig *fig=convexFigNew();
	fig->index=index;
	convexSpaceCreateVert(&tmpSpace, convexAttached->vertices[index]);
	convexSpaceAssign(tmpSpace, fig);
	fig->hash=rand();
	convexShadow[0]=safeRealloc(convexShadow[0], convexAttached->count[0]*sizeof(struct convexFig *));
	convexShadow[0][index]=fig;
	if (convexHull)
		convexInteractStart("Updating convex hull...");
	else
		convexInteractStart(0);
	convexHullVertAdd(fig);
	if (convexHull)
		convexInteractStop("Convex hull has been updated");
	else
		convexInteractStop(0);
	if (convexHull) {
		exportHull();
	}
	DEBUG_HULL_VERBOSE(DEBUG_HULL_DOT(convexFigPrint();))
}

void convexVertexRm(int index) {
	int i;
	if (convexHull)
		convexInteractStart("Updating convex hull...");
	else
		convexInteractStart("Removing appropriate faces...");
	convexHullVertRm(convexShadow[0][index]);
	convexSpaceUnassign(convexShadow[0][index]);
	convexFigDelete(convexShadow[0][index]);
	for (i=index; i<convexAttached->count[0]-1; i++) {
		convexShadow[0][i]=convexShadow[0][i+1];
		convexFigTouch(convexShadow[0][i]);
		convexShadow[0][i]->index=i;
	}
	convexShadow[0]=safeRealloc(convexShadow[0], (convexAttached->count[0]-1)*sizeof(struct convexFig *));
	if (convexHull)
		convexInteractStop("Convex hull updated");
	else
		convexInteractStop("Topology of the figure updated");
	if (!convexInteractAborted)
		exportHull();
	DEBUG_HULL_VERBOSE(DEBUG_HULL_DOT(convexFigPrint();))
}

void convexVertexMove(int index, GLdouble *pos) {
	if (convexHull) {
		convexInteractStart("Updating convex hull...");
		// convexHullVertRm(convexShadow[0][index]);
	} else {
		convexInteractStart("Breaking appropriate faces...");
		convexHullBreakNearVert(convexShadow[0][index]);
		convexInteractStop("Topology of the figure updated");
	}
	convexSpaceUnassign(convexShadow[0][index]);
	convexSpaceCreateVert(&tmpSpace, pos);
	convexSpaceAssign(tmpSpace, convexShadow[0][index]);
	DEBUG_HULL_VERBOSE(DEBUG_HULL_DOT(convexFigPrint();))
	if (convexHull) {
		// convexHullVertAdd(convexShadow[0][index]);
		convexHullCreate();
		convexInteractStop("Convex hull updated");
	}
	if (convexInteractAborted) {
		convexDetach();
	} else {
		exportHull();
	}
};
