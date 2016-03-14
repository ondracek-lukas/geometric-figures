// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexHull.h"

#include <stdio.h>
#include <stdbool.h>

#include "convex.h"
#include "convexFig.h"
#include "convexHullUtil.h"
#include "convexSpace.h"
#include "convexLoopDetect.h"
#include "convexInteract.h"
#include "figure.h"
#include "debug.h"

bool convexHull=0;

static struct convexSpace *tmpSpace=0;

void convexHullVertAdd(struct convexFig *vert) {
	DEBUG_HULL(printf("\n-- add vert %02d-%08x --\n", vert->index, vert->hash);)
	convexLoopDetectReset();
	if (convexInteractAborted)
		return;
	struct convexFigList *list=0;
	struct convexFig *fig;
	convexFigListAdd(&convexFreeVertices, vert);
	if (!convexHull)
		return;
	if (!convexFigure)
		convexFigListAdd(&convexFigure, vert);
	else {
		if (convexSpaceContains(convexFigure->fig->space, vert->space)) {
			convexFigListAdd(&list, vert);
			convexHullUtilExpand(convexFigure->fig, list, 0);
			convexFigListRm(&list);
		} else {
			convexHullUtilExpandDim(convexFigListRm(&convexFigure), vert, &fig, 0);
			convexFigListAdd(&convexFigure, fig);
		}
	}
	DEBUG_LOOP(convexLoopDetectPrint();)
}

void convexHullVertRm(struct convexFig *vert) {
	DEBUG_HULL(printf("\n--  rm vert %02d-%08x --\n", vert->index, vert->hash);)
	convexLoopDetectReset();
	if (convexInteractAborted)
		return;
	struct convexFigList *list=0;
	struct convexFig *fig;
	if (!vert->parents)
		convexFigListRmFig(&convexFigure, vert);
	else {
		int figureBroken=(convexFigListLen(convexFigure)>1);
		convexFigGetLayer(vert,
			convexFigure->fig->space->dim-!figureBroken,
			convexFigMarkIdTrue, convexFigMarkIdTrue, &list);
		figureBroken=!figureBroken && list;
		while (list) {
			convexFigListRmFig(&convexFigure, list->fig);
			convexFigDestroy(convexFigListRm(&list));
		}
		if (convexHull)
			figureBroken=!convexFigure->fig->boundary->next; // only one face remains
		if (figureBroken) {
			DEBUG_HULL_VERBOSE(printf("Decreasing dimen\n");)
			fig=convexFigListRm(&convexFigure);
			convexFigListCopy(fig->boundary, &convexFigure, convexFigMarkIdTrue);
			while (fig->boundary)
				convexFigBoundaryDetach(fig, fig->boundary->fig);
			convexFigDestroy(fig);
		} else if (convexHull) {
				DEBUG_HULL_VERBOSE(printf("Closing hull\n");)
				convexHullUtilAddInconsistent(&list, convexFigure->fig);
				convexHullUtilRepair(&list, 0);
		}
		DEBUG_LOOP(convexLoopDetectPrint();)
	}
	convexFigListRmFig(&convexFreeVertices, vert);

	if (convexHull) {
		DEBUG_HULL_VERBOSE(printf("Adding free vertices\n");)
		list=convexFreeVertices;
		convexFreeVertices=0;
		while (list)
			convexHullVertAdd(convexFigListRm(&list));
	}
}

void convexHullCreate() {
	convexLoopDetectReset();
	if (convexInteractAborted)
		return;
	struct convexFigList *list=0;
	convexHullDestroy();
	if (convexFreeVertices) {
		convexFigListCopy(convexFreeVertices, &list, convexFigMarkIdTrue);
		convexFigListAdd(&convexFigure, convexHullUtilCreate(list, 0));
		convexFigListDestroy(&list);
	}
}

void convexHullUpdate() {
	int i, dim;
	struct convexFigList **pList, *figs=0, *figs2=0;
	if (convexFigListLen(convexFigure)==1) {
		convexLoopDetectReset();
		convexSpaceCopy(convexShadow[0][0]->space, &tmpSpace);
		for (i=1; i<convexAttached->count[0]; i++)
			if (!convexSpaceContains(tmpSpace, convexShadow[0][i]->space))
				convexSpaceExpand(tmpSpace, convexShadow[0][i]->space);
		if (convexSpaceEq(tmpSpace, convexFigure->fig->space)) {
			for (dim=1; dim<convexFigure->fig->space->dim; dim++) {
				convexFigGetLayer(convexFigure->fig, dim, convexFigMarkIdTrue, convexFigMarkIdTrue, &figs);
				pList=&figs;
				while (*pList) {
					convexSpaceGetFigs((*pList)->fig->space, &figs2);
					convexFigListRmFig(&figs2, (*pList)->fig);
					if (figs2) {
						while ((*pList)->fig->parents) {
							convexFigBoundaryAttach((*pList)->fig->parents->fig, figs2->fig);
							convexFigBoundaryDetach((*pList)->fig->parents->fig, (*pList)->fig);
						}
						while ((*pList)->fig->boundary) {
							convexFigBoundaryAttach(figs2->fig, (*pList)->fig->boundary->fig);
							convexFigBoundaryDetach((*pList)->fig, (*pList)->fig->boundary->fig);
						}
						convexFigDestroy(convexFigListRm(pList));
						convexFigListDestroy(&figs2);
					} else
						pList=&(*pList)->next;
				}
				convexHullUtilRepair(&figs, &figs2);
				convexFigListDestroy(&figs2);
				if (convexInteractAborted)
					return;
			}
			convexFigListAdd(&figs, convexFigure->fig);
			convexHullUtilRepair(&figs, 0);
			convexFigListCopy(convexFreeVertices, &figs, convexFigMarkIdTrue);
			convexHullUtilExpand(convexFigure->fig, figs, 0);
			convexFigListDestroy(&figs);
			return;
		}
	}
	convexHullCreate();
}

void convexHullBreakNearVert(struct convexFig *vertIn) {
	struct convexFigList *verticesCenter=0, *verticesCenterFace=0, *edgesCenter=0;
	struct convexFigList *edgesIn=0, *facesIn=0, *faces=0;
	struct convexFigList *list;
	struct convexFig *edgeCenter, *faceIn, *faceOut;
	unsigned int hash;
	int vertCount;
	int dim;
	if (convexInteractAborted)
		return;
	if (!convexFigure)
		return;
	convexLoopDetectDisable();

	convexFigGetLayer(vertIn, 1, convexFigMarkIdTrue, convexFigMarkIdTrue, &edgesIn);
	convexFigMarkReset(convexFigMarkIdHull);
	for (list=edgesIn; list; list=list->next)
		convexFigGetLayer(list->fig, 0, convexFigMarkIdTrue, convexFigMarkIdHull, 0);
	convexFigMarkClear(vertIn, convexFigMarkIdHull);

	for (dim=2; dim<convexAttached->dim; dim++) {
		// state:
			// verticesCenter (dim-2) are hull-marked
			// edgesIn (dim-1) list exist
			// vertIn - still the same vertex
		convexFigGetLayer(vertIn, dim, convexFigMarkIdTrue, convexFigMarkIdTrue, &faces);
		for (; faces; convexFigListRm(&faces)) {
			convexFigTouch(faces->fig);
			convexFigGetLayer(faces->fig, dim-2, convexFigMarkIdHull, convexFigMarkIdTrue, &verticesCenterFace);
			convexFigMarkReset(convexFigMarkIdHash);
			hash=0;
			vertCount=0;
			for (list=verticesCenterFace; list; list=list->next)
				vertCount+=convexFigHashCalc(list->fig, &hash);
			if ((edgeCenter=convexFigHashFind(verticesCenterFace->fig->parents, hash, vertCount))) {
				faceIn=faces->fig;
			} else {
				edgeCenter=convexFigNew();
				edgeCenter->space->dim=dim-1;
				edgeCenter->hash=hash;
				for (list=verticesCenterFace; list; list=list->next)
					convexFigBoundaryAttach(edgeCenter, list->fig);
				convexSpaceCreate(&tmpSpace, edgeCenter);
				convexSpaceAssign(tmpSpace, edgeCenter);
				faceOut=faces->fig;
				faceIn=convexFigNew();
				convexSpaceCopy(faceOut->space, &tmpSpace);
				convexSpaceAssign(tmpSpace, faceIn);
				for (list=edgesIn; list; list=list->next)
					if (convexFigBoundaryDetach(faceOut, list->fig))
						convexFigBoundaryAttach(faceIn, list->fig);
				convexFigBoundaryAttach(faceOut, edgeCenter);
				convexFigBoundaryAttach(faceIn, edgeCenter);
				for (list=faceOut->parents; list; list=list->next)
					convexFigBoundaryAttach(list->fig, faceIn);
				if (convexFigListContains(convexFigure, faceOut))
					convexFigListAdd(&convexFigure, faceIn);
			}
			convexFigListDestroy(&verticesCenterFace);
			convexFigListAdd(&edgesCenter, edgeCenter);
			convexFigListAdd(&facesIn, faceIn);
			convexInteractUpdate();
			if (convexInteractAborted)
				break;
		}
		convexFigListDestroy(&edgesIn);
		edgesIn=facesIn;
		facesIn=0;
		verticesCenter=edgesCenter;
		edgesCenter=0;
		convexFigMarkReset(convexFigMarkIdHull);
		convexFigListMarkSet(verticesCenter, convexFigMarkIdHull);
		convexFigListDestroy(&verticesCenter);
		convexInteractUpdate();
		if (convexInteractAborted)
			break;
	}
}

void convexHullDestroy() {
	while (convexFigure)
		convexFigDestroy(convexFigListRm(&convexFigure));
}
