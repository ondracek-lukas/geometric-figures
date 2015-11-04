// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexFig.h"

#include <stdlib.h>

#include "figure.h"
#include "safe.h"
#include "convex.h"
#include "convexLoopDetect.h"
#include "debug.h"
#include "convexSpace.h"

struct convexFigList *convexFigFreed=0;

static int hashFigEqual(struct convexFig *fig);

struct convexFig *convexFigNew() {
	struct convexFig *fig;
	int i;
	if (convexFigFreed) {
		fig=convexFigListRm(&convexFigFreed);
		if (fig->space->dim!=convexAttached->dim) {
			free(fig->space->pos);
			free(fig->space->normal);
			fig->space->dim=0;
		}
	} else {
		fig=safeMalloc(sizeof(struct convexFig));
		fig->space=safeMalloc(sizeof(struct convexSpace));
		fig->space->dim=0;
	}

	if (fig->space->dim)
		fig->space->dim=0;
	else {
		fig->space->pos=safeMalloc(convexAttached->dim*sizeof(GLfloat));
		fig->space->normal=safeMalloc(convexAttached->dim*sizeof(GLfloat));
	}
	fig->index=-1;
	fig->hash=0;
	for (i=0; i<convexFigMarkCount; i++)
		fig->mark[i]=0;
	fig->parents=0;
	fig->boundary=0;
	fig->space->normalPos=0;
	fig->space->ortBasis=0;
	return fig;
}

void convexFigDelete(struct convexFig *fig) {
	DEBUG_HULL(
		if (fig->space->ortBasis)
			printf("err: basis exists on delete\n");
		struct convexFigList *list=0;
		convexFigBstGetAll(convexSpaces, &list);
		while (list) {
			if (list->fig==fig)
				printf("err: fig space assigned on delete\n");
			convexFigListRm(&list);
		}
		if (fig->index<-1)
			printf("Err: fig deleted twice\n");
		fig->index=-fig->index-100;
	)

	fig->space->dim=convexAttached->dim;
	convexFigListAdd(&convexFigFreed, fig);
}

void convexFigBoundaryAttach(struct convexFig *parent, struct convexFig *child) {
	convexFigBoundaryDetach(parent, child);
	if (!child->parents)
		convexFigListRmFig(&convexFreeVertices, child);
	convexFigListAdd(&(parent->boundary), child);
	convexFigListAdd(&(child->parents), parent);
}

int convexFigBoundaryDetach(struct convexFig *parent, struct convexFig *child) {
	int ret=0;
	convexLoopDetectAction(parent, child);
	convexFigListRmFig(&parent->boundary, child);
	ret=convexFigListRmFig(&child->parents, parent);
	if (ret && (child->space->dim==0) && (!child->parents))
		convexFigListAdd(&convexFreeVertices, child);
	return ret;
}

void convexFigDestroy(struct convexFig *fig) {
	struct convexFig *fig2;
	convexFigTouch(fig);
	while (fig->parents)
		convexFigBoundaryDetach(fig->parents->fig, fig);
	while (fig->boundary) {
		fig2=fig->boundary->fig;
		convexFigBoundaryDetach(fig, fig2);
		if (!fig2->parents)
			convexFigDestroy(fig2);
	}
	if (fig->space->dim>0) {
		convexSpaceUnassign(fig);
		convexFigDelete(fig);
	}
}

void convexFigTouch(struct convexFig *fig) {
	struct convexFigList *list;
	if (fig->index<0)
		return;
	for (list=fig->parents; list; list=list->next)
		convexFigTouch(list->fig);
	if (fig->space->dim>0) {
		convexShadow[fig->space->dim][fig->index]=0;
		fig->index=-1;
	}
}

int convexFigGetLayer(struct convexFig *fig, int dim, enum convexFigMarkId filterMark, enum convexFigMarkId skipMark, struct convexFigList **layer) {
	// skipMarked figures are skipped, the others are marked; not-skipping if mark-true
	// non-filterMarked figures of given dim are skipped
	if (skipMark==convexFigMarkIdTrue)
		convexFigMarkReset(skipMark=convexFigMarkIdLayer);

	struct convexFigList *list;
	int count=0;

	if (convexFigMarkGet(fig, skipMark))
		return 0;
	convexFigMarkSet(fig, skipMark);

	if (fig->space->dim==dim) {
		if (convexFigMarkGet(fig, filterMark)) {
			if (layer)
				convexFigListAdd(layer, fig);
			return 1;
		} else
			return 0;
	} else if (fig->space->dim>dim) {
		list=fig->boundary;
		while (list) {
			count+=convexFigGetLayer(list->fig, dim, filterMark, skipMark, layer);
			list=list->next;
		}
	} else if (fig->space->dim<dim) {
		list=fig->parents;
		while (list) {
			count+=convexFigGetLayer(list->fig, dim, filterMark, skipMark, layer);
			list=list->next;
		}
	}
	return count;
}

int convexFigHashCalc(struct convexFig *fig, unsigned int *hash) {
	// hash-marked figures are skipped, the others are marked
	struct convexFigList *vertices=0;
	int vertCount=convexFigGetLayer(fig, 0, convexFigMarkIdTrue, convexFigMarkIdHash, &vertices);
	while (vertices)
		*hash^=convexFigListRm(&vertices)->hash;
	return vertCount;
}

static int hashFigEqual(struct convexFig *fig) {
	// all vertices hash-marked ? returns count : returns 0
	struct convexFigList *vertices=0;
	int vertCount=convexFigGetLayer(fig, 0, convexFigMarkIdTrue, convexFigMarkIdTrue, &vertices);
	while (vertices)
		if (!convexFigMarkGet(convexFigListRm(&vertices), convexFigMarkIdHash)) {
			convexFigListDestroy(&vertices);
			return 0;
		}
	return vertCount;
}

struct convexFig *convexFigHashFind(struct convexFigList *list, unsigned int hash, int vertCount) {
	// returns figure from list with given hash and all vertices (given count) hash-marked or 0
	while (list) {
		if ((list->fig->hash==hash) && (hashFigEqual(list->fig)==vertCount))
			return list->fig;
		list=list->next;
	}
	return 0;
}
