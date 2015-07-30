// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convex.h"

#include "safe.h"
#include "convexFig.h"
#include "convexLoopDetect.h"
#include "convexSpace.h"
#include "convexInteract.h"
#include "convexHull.h"
#include "debug.h"
#include "figure.h"

struct convexFigList *convexFigure=0;
struct convexFigList *convexFreeVertices=0;
struct convexFig ***convexShadow=0;
bool convexAttached=0;

static struct convexSpace *tmpSpace=0;
static void exportHull();

void convexAttach() {
	struct convexFigList *wrongDimList=0;
	struct convexFig *fig;
	int i, j, dim;
	convexLoopDetectDisable();
	if (convexAttached)
		convexDetach();
	if (figureData.dim<0)
		return;
	convexShadow=safeMalloc((figureData.dim+1)*sizeof(struct convexFig**));
	for (i=0; i<=figureData.dim; i++)
		convexShadow[i]=safeMalloc(figureData.count[i]*sizeof(struct convexFig*));
	for (i=0; i<figureData.count[0]; i++) {
		fig=convexFigNew();
		fig->index=i;
		convexSpaceCreateVert(&tmpSpace, figureData.vertices[i]);
		convexSpaceAssign(tmpSpace, fig);
		fig->hash=rand();
		convexShadow[0][i]=fig;
	}
	for (dim=1; dim<=figureData.dim; dim++) {
		for (i=0; i<figureData.count[dim]; i++) {
			fig=convexFigNew();
			fig->space->dim=dim;
			fig->index=i;
			convexShadow[dim][i]=fig;
			for (j=1; j<=figureData.boundary[dim][i][0]; j++)
				if (convexShadow[dim-1][figureData.boundary[dim][i][j]])
					convexFigBoundaryAttach(fig, convexShadow[dim-1][figureData.boundary[dim][i][j]]);
			convexSpaceCreate(&tmpSpace, fig);
			convexSpaceAssign(tmpSpace, fig);
			if (fig->space->dim != dim) {
				fig->space->dim=dim;
				if (convexHull)
					convexFigListAdd(&wrongDimList, fig);
			}
			convexFigMarkReset(convexFigMarkIdHash);
			convexFigHashCalc(fig, &fig->hash);
		}
		while (wrongDimList)
			convexFigDestroy(convexFigListRm(&wrongDimList));
	}
	for (dim=figureData.dim; (dim>0) && !convexFigure; dim--)
		for (i=0; i<figureData.count[dim]; i++)
			if (convexShadow[dim][i])
				convexFigListAdd(&convexFigure, convexShadow[dim][i]);
	for (i=0; i<figureData.count[0]; i++)
		if (!convexShadow[0][i]->parents)
			convexFigListAdd(&convexFreeVertices, convexShadow[0][i]);
	convexAttached=1;

	convexUpdateHull();
}


void convexUpdateHull() {
	if (convexHull) {
		convexInteractStart("Checking and updating convex hull...");
		convexHullUpdate();
		convexInteractStop("Convex hull has been checked and updated...");
		exportHull();
	}
}

void convexDestroyHull() {
	convexHullDestroy();
	exportHull();
}

static void exportHull() {
	int dim, count, i, j;
	struct convexFigList *list1, *list2=0;
	for (dim=1; dim<=figureData.dim; dim++) {
		count=0;
		convexFigMarkReset(convexFigMarkIdLayer);
		for (list1=convexFigure; list1; list1=list1->next)
			count+=convexFigGetLayer(list1->fig, dim, convexFigMarkIdTrue, convexFigMarkIdLayer, &list2);
		DEBUG_HULL_VERBOSE(printf("count[%d]=%d\n", dim, count);)
		if (count!=figureData.count[dim]) {
			for (i=count; i<figureData.count[dim]; i++)
				if (convexShadow[dim][i])
					convexFigTouch(convexShadow[dim][i]);
			figureData.boundary[dim]=safeRealloc(figureData.boundary[dim], count*sizeof(GLint *));
			convexShadow[dim]=safeRealloc(convexShadow[dim], count*sizeof(struct convexFig *));
		}
		for (i=0; i<count; i++) {
			if ((i<figureData.count[dim]) && convexShadow[dim][i]) {
				DEBUG_HULL_VERBOSE(printf("Skipping shadow[%d][%d] - fig index %d\n", dim, i, convexShadow[dim][i]->index);)
				continue;
			}
			while (list2->fig->index>=0) {
				DEBUG_HULL_VERBOSE(printf("Skipping fig index %d-%d\n", dim, list2->fig->index);)
				convexFigListRm(&list2);
			}
			list2->fig->index=i;
			convexShadow[dim][i]=list2->fig;
			if (i<figureData.count[dim])
				free(figureData.boundary[dim][i]);
			list1=list2->fig->boundary;
			j=convexFigListLen(list1);
			figureData.boundary[dim][i]=safeMalloc((j+1)*sizeof(GLint));
			figureData.boundary[dim][i][0]=j;
			DEBUG_HULL_VERBOSE(printf("%d-%d-%d: %d\n", dim, i, 0, j);)
			for (j=1; list1; j++, list1=list1->next) {
				figureData.boundary[dim][i][j]=list1->fig->index;
				DEBUG_HULL_VERBOSE(printf("%d-%d-%d: %d\n", dim, i, j, list1->fig->index);)
			}
		}
		DEBUG_HULL_VERBOSE(
			while (list2 && (list2->fig->index>=0)) {
				printf("Skipping fig index %d-%d\n", dim, list2->fig->index);
				convexFigListRm(&list2);
			}
			if (list2)
				safeExitErr("convex export error");)
		convexFigListDestroy(&list2);
		figureData.count[dim]=count;
	}
	DEBUG_HULL(
		for (dim=1; dim<=figureData.dim; dim++)
			for (i=0; i<figureData.count[dim]; i++)
				for (j=1; j<=figureData.boundary[dim][i][0]; j++)
					if (figureData.boundary[dim][i][j]<0)
						safeExitErr("convex export error");)
	figureBoundaryChanged();
}

void convexDetach() {
	int dim;
	struct convexFigList *list=0;
	if (!convexAttached)
		return;
	convexHullDestroy();
	for (dim=0; dim<=figureData.dim; dim++)
		free(convexShadow[dim]);
	free(convexShadow);
	while (convexFreeVertices) {
		convexSpaceUnassign(convexFreeVertices->fig);
		convexFigDelete(convexFigListRm(&convexFreeVertices));
	}
	if (convexFigBstGetAll(convexSpaces, &list)) // needed after abortion
		while (list) {
			convexSpaceUnassign(list->fig);
			convexFigDelete(convexFigListRm(&list));
		}
	convexShadow=0;
	convexAttached=0;
}

void convexVertexAdd(int index) {
	struct convexFig *fig=convexFigNew();
	fig->index=index;
	convexSpaceCreateVert(&tmpSpace, figureData.vertices[index]);
	convexSpaceAssign(tmpSpace, fig);
	fig->hash=rand();
	convexShadow[0]=safeRealloc(convexShadow[0], figureData.count[0]*sizeof(struct convexFig *));
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
	if (convexHull || convexInteractAborted)
		exportHull();
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
	for (i=index; i<figureData.count[0]-1; i++) {
		convexShadow[0][i]=convexShadow[0][i+1];
		convexFigTouch(convexShadow[0][i]);
		convexShadow[0][i]->index=i;
	}
	convexShadow[0]=safeRealloc(convexShadow[0], (figureData.count[0]-1)*sizeof(struct convexFig *));
	if (convexHull)
		convexInteractStop("Convex hull has been updated");
	else
		convexInteractStop("The appropriate faces have been removed");
	exportHull();
	DEBUG_HULL_VERBOSE(DEBUG_HULL_DOT(convexFigPrint();))
}

void convexVertexMove(int index, GLfloat *pos) {
	if (convexHull) {
		convexInteractStart("Updating convex hull...");
		convexHullVertRm(convexShadow[0][index]);
	} else {
		convexInteractStart("Breaking appropriate faces...");
		convexHullBreakNearVert(convexShadow[0][index]);
		convexInteractStop("The appropriate faces have been broken");
	}
	convexSpaceUnassign(convexShadow[0][index]);
	convexSpaceCreateVert(&tmpSpace, pos);
	convexSpaceAssign(tmpSpace, convexShadow[0][index]);
	DEBUG_HULL_VERBOSE(DEBUG_HULL_DOT(convexFigPrint();))
	if (convexHull) {
		convexHullVertAdd(convexShadow[0][index]);
		convexInteractStop("Convex hull has been updated");
	}
	exportHull();
};
