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
	convexFigListAdd(&convexFreeVertices, vert);
	if (convexHull) {
		convexHullCreate();
	}
	/* Update only, sometimes causes errors
	if (convexHull) {
		if (!convexFigure) {
			convexFigListAdd(&convexFigure, vert);
		} else {
			convexFigMarkReset(convexFigMarkIdHullProcessed);
			struct convexFigList *vertices=0;
			convexFigListAdd(&vertices, vert);
			if (convexSpaceContains(convexFigure->fig->space, vert->space)) {
				convexHullUtilExpand(convexFigure->fig, vertices);
			} else {
				struct convexFig *fig;
				// calculate newSpace
				fig=convexHullUtilExpandDim(convexFigListRm(&convexFigure), vertices, newSpace);
				convexFigListAdd(&convexFigure, fig);
			}
			convexFigListDestroy(&vertices);
		}
		DEBUG_LOOP(convexLoopDetectPrint();)
	}
	*/
}

static void removeFig(struct convexFig *fig);

void convexHullVertRm(struct convexFig *vert) {
	DEBUG_HULL(printf("\n--  rm vert %02d-%08x --\n", vert->index, vert->hash);)
	convexLoopDetectReset();
	if (convexInteractAborted)
		return;
	if (convexHull) {
		removeFig(vert);
		convexFigListRmFig(&convexFreeVertices, vert);
		convexHullCreate();
	} else {
		convexHullBreakNearVert(vert);
		removeFig(vert);
		convexFigListRmFig(&convexFreeVertices, vert);
	}
}

static void removeFig(struct convexFig *fig) {
	while (fig->parents) {
		struct convexFig *parent=fig->parents->fig;
		convexFigBoundaryDetach(parent, fig);
		if (!convexHull || parent->parents || (convexFigListLen(parent->boundary)<=1)) {
			while (parent->boundary) {
				struct convexFig *fig2=parent->boundary->fig;
				convexFigBoundaryDetach(parent, fig2);
				if (!fig2->parents && (!convexHull || !parent->parents)) {
					convexFigListAdd(&convexFigure, fig2);
				}
			}
			removeFig(parent);
		}
	}
	convexFigListRmFig(&convexFigure, fig);
	convexFigDestroy(fig);
}

void convexHullCreate() {
	convexLoopDetectReset();
	if (convexInteractAborted)
		return;
	convexHullDestroy();
	convexFigMarkResetHard(convexFreeVertices);
	if (convexFreeVertices) {
		struct convexFigList *list=0;
		convexFigMarkReset(convexFigMarkIdHullProcessed);
		convexFigListCopy(convexFreeVertices, &list, convexFigMarkIdTrue);
		convexFigListAdd(&convexFigure,
			convexHullUtilCreate(list));
		convexFigListDestroy(&list);
	}
}

void convexHullBreakNearVert(struct convexFig *vertIn) {
	struct convexFigList *peaksCenter=0, *peaksCenterFace=0, *ridgesCenter=0;
	struct convexFigList *ridgesIn=0, *facesIn=0, *faces=0;
	struct convexFigList *list;
	struct convexFig *ridgeCenter, *faceIn, *faceOut;
	unsigned int hash;
	int vertCount;
	int dim;
	if (convexInteractAborted)
		return;
	if (!convexFigure)
		return;
	convexLoopDetectDisable();

	convexFigGetLayer(vertIn, 1, convexFigMarkIdTrue, convexFigMarkIdTrue, &ridgesIn);
	convexFigMarkReset(convexFigMarkIdHull);
	for (list=ridgesIn; list; list=list->next)
		convexFigGetLayer(list->fig, 0, convexFigMarkIdTrue, convexFigMarkIdHull, 0);
	convexFigMarkClear(vertIn, convexFigMarkIdHull);

	for (dim=2; dim<convexAttached->dim; dim++) {
		// state:
			// peaksCenter (dim-2) are hull-marked
			// ridgesIn (dim-1) list exist
			// vertIn - still the same vertex
		convexFigGetLayer(vertIn, dim, convexFigMarkIdTrue, convexFigMarkIdTrue, &faces);
		for (; faces; convexFigListRm(&faces)) {
			convexFigTouch(faces->fig);
			convexFigGetLayer(faces->fig, dim-2, convexFigMarkIdHull, convexFigMarkIdTrue, &peaksCenterFace);
			convexFigMarkReset(convexFigMarkIdHash);
			hash=0;
			vertCount=0;
			for (list=peaksCenterFace; list; list=list->next)
				vertCount+=convexFigHashCalc(list->fig, &hash);
			if ((ridgeCenter=convexFigHashFind(hash, dim-1, vertCount))) {
				faceIn=faces->fig;
			} else {
				ridgeCenter=convexFigNew();
				ridgeCenter->space->dim=dim-1;
				ridgeCenter->hash=hash;
				convexFigMarkReset(convexFigMarkIdLayer);
				for (list=peaksCenterFace; list; list=list->next) {
					convexFigGetLayer(list->fig, 0, convexFigMarkIdTrue, convexFigMarkIdLayer, &ridgeCenter->vertices);
				}
				convexFigHashAdd(ridgeCenter);
				for (list=peaksCenterFace; list; list=list->next)
					convexFigBoundaryAttach(ridgeCenter, list->fig);
				convexSpaceCreate(&tmpSpace, ridgeCenter);
				if (ridgeCenter->space->dim != tmpSpace->dim) {
					convexInteractAbort("Error: generated figure has wrong dimension (BreakFaces)");
				}
				convexSpaceAssign(tmpSpace, ridgeCenter);
				faceOut=faces->fig;
				faceIn=convexFigNew();
				convexSpaceCopy(faceOut->space, &tmpSpace);
				convexSpaceAssign(tmpSpace, faceIn);
				for (list=ridgesIn; list; list=list->next)
					if (convexFigBoundaryDetach(faceOut, list->fig))
						convexFigBoundaryAttach(faceIn, list->fig);
				convexFigBoundaryAttach(faceOut, ridgeCenter);
				convexFigBoundaryAttach(faceIn, ridgeCenter);
				for (list=faceOut->parents; list; list=list->next)
					convexFigBoundaryAttach(list->fig, faceIn);
				if (convexFigListContains(convexFigure, faceOut))
					convexFigListAdd(&convexFigure, faceIn);
			}
			convexFigListDestroy(&peaksCenterFace);
			convexFigListAdd(&ridgesCenter, ridgeCenter);
			convexFigListAdd(&facesIn, faceIn);
			convexInteractUpdate();
			if (convexInteractAborted)
				break;
		}
		convexFigListDestroy(&ridgesIn);
		ridgesIn=facesIn;
		facesIn=0;
		peaksCenter=ridgesCenter;
		ridgesCenter=0;
		convexFigMarkReset(convexFigMarkIdHull);
		convexFigListMarkSet(peaksCenter, convexFigMarkIdHull);
		convexFigListDestroy(&peaksCenter);
		convexInteractUpdate();
		if (convexInteractAborted)
			break;
	}
}

void convexHullDestroy() {
	while (convexFigure)
		convexFigDestroy(convexFigListRm(&convexFigure));
}
