// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexHullUtil.h"

#include <stdio.h>
#include <stdbool.h>

#include "convexSpace.h"
#include "convexFig.h"
#include "convexInteract.h"
#include "figure.h"
#include "debug.h"

struct convexSpace *tmpSpace;

bool convexHullUtilExpandDim(struct convexFig *fig, struct convexFig *vert, struct convexFig **newFig, struct convexFigList **inconsistent) {
	// returns whether new fig is larger than it should be
	// inconsistent are (dim+1)-figs
	unsigned int hash=0;
	bool larger=0;
	int vertCount;
	struct convexFigList *list=0, *list2=0;
	DEBUG_HULL_VERBOSE(printf("-ExpandDim(%d) - enter\n", fig->space->dim+1);)

	convexFigMarkReset(convexFigMarkIdHash);
	vertCount=convexFigHashCalc(fig, &hash);
	vertCount+=convexFigHashCalc(vert, &hash);
	if (*newFig=convexFigHashFind(fig->parents, hash, vertCount)) {
		DEBUG_HULL_VERBOSE(printf("-ExpandDim(%d) - exit (fig exists)\n", fig->space->dim+1);)
		return 0;
	}
	
	convexSpaceCopy(fig->space, &tmpSpace);
	convexSpaceExpand(tmpSpace, vert->space);
	if (convexSpaceGetFigs(tmpSpace, &list)) {
		*newFig=list->fig;
		convexFigListDestroy(&list);
		convexFigGetLayer(fig, 0, convexFigMarkIdTrue, convexFigMarkIdTrue, &list);
		convexFigListAdd(&list, vert);
		convexHullUtilExpand(*newFig, list, inconsistent);
		convexFigListDestroy(&list);
		DEBUG_HULL_VERBOSE(printf("-ExpandDim(%d) - exit (space exists)\n", fig->space->dim+1);)
		return 1;
	}

	*newFig=convexFigNew();
	(*newFig)->hash=hash;
	convexSpaceAssign(tmpSpace, *newFig);
	convexFigBoundaryAttach(*newFig, fig);
	if (fig->space->dim==0) {
		convexFigBoundaryAttach(*newFig, vert);
	} else {
		for (list=fig->boundary; list; list=list->next) {
			larger|=convexHullUtilExpandDim(list->fig, vert, &fig, &list2);
			convexFigBoundaryAttach(*newFig, fig);
		}
		if (larger) {
			convexHullUtilAddInconsistent(&list2, *newFig);
			convexHullUtilRepair(&list2, inconsistent);
		}
	}
	
	convexFigGetLayer(*newFig, 0, convexFigMarkIdTrue, convexFigMarkIdTrue, &list);
	convexSpaceCenterPos((*newFig)->space, list);
	convexFigListDestroy(&list);
	DEBUG_HULL_VERBOSE(printf("-ExpandDim(%d) - exit (created)\n", fig->space->dim+1);)
	return larger;
}

bool convexHullUtilExpand(struct convexFig *fig, struct convexFigList *vertices, struct convexFigList **inconsistent) {
	// returns whether expansion was needed
	// inconsistent are (dim+1)-figs
	DEBUG_HULL_VERBOSE(printf("-Expand(%d) - enter\n", fig->space->dim);)
	struct convexFigList *list=0, *list2=0;
	convexHullUtilNormalsCalc(fig);
	convexFigMarkSet(fig, convexFigMarkIdHullInconsistent);
	if (!convexHullUtilWrongFacesRm(fig, vertices, &list2)) {
		convexFigMarkClear(fig, convexFigMarkIdHullInconsistent);
		DEBUG_HULL_VERBOSE(printf("-Expand(%d) - exit (no change)\n", fig->space->dim);)
		return 0;
	}

	convexFigTouch(fig);
	convexFigMarkReset(convexFigMarkIdHull);
	convexFigListCopy(vertices, &list, convexFigMarkIdHull);
	convexFigGetLayer(fig, 0, convexFigMarkIdTrue, convexFigMarkIdHull, &list);
	convexHullUtilComplete(fig, &list, &list2);
	convexFigMarkClear(fig, convexFigMarkIdHullInconsistent);
	convexFigListDestroy(&list);
	convexHullUtilRepair(&list2, inconsistent);
	DEBUG_HULL_VERBOSE(printf("-Expand(%d) - exit\n", fig->space->dim);)
	return 1;
}

void convexHullUtilRepair(struct convexFigList **pList, struct convexFigList **inconsistent) {
	// repairs list of figs, returns higher-dim inconsistent figs
	struct convexFigList *vertices=0, *list;
	struct convexFig *fig;
	while (*pList) {
		DEBUG_HULL_VERBOSE(printf("-Repair(%d) - enter (one fig)\n", (*pList)->fig->space->dim);)
		fig=convexFigListRm(pList);
		convexFigTouch(fig);
		for (list=fig->parents; list; list=list->next)
			convexHullUtilAddInconsistent(inconsistent, list->fig);
		convexFigGetLayer(fig, 0, convexFigMarkIdTrue, convexFigMarkIdTrue, &vertices);
		convexSpaceCenterPos(fig->space, vertices);
		convexHullUtilNormalsCalc(fig);
		convexHullUtilWrongFacesRm(fig, vertices, pList);
		convexHullUtilComplete(fig, &vertices, pList);
		convexFigMarkClear(fig, convexFigMarkIdHullInconsistent);
		convexFigListDestroy(&vertices);
		DEBUG_HULL_VERBOSE(printf("-Repair(%d) - exit (one fig)\n", fig->space->dim);)
		if (convexInteractAborted)
			convexFigListDestroy(pList);
	}
}


int convexHullUtilWrongFacesRm(struct convexFig *fig, struct convexFigList *vertices, struct convexFigList **inconsistent) {
	// returns count of removed faces
	// inconsistent are neighbours (same dim)
	// normals of faces needed
	struct convexFigList **pList;
	struct convexFigList *list, *list2;
	int removed=0;
	DEBUG_HULL_VERBOSE(printf("-FacesRm(%d) - enter\n", fig->space->dim);)
	for (pList=&fig->boundary; *pList; !list && (pList=&(*pList)->next)) {
		for (list=vertices; list; list=list->next) {
			if (figureDistCmpZero(convexSpaceOrientedDist((*pList)->fig->space, list->fig->space), figureDistCmpToleranceDefault)>0) {
				DEBUG_HULL(printf("Removing %dD-%02d-%08x from %dD-%02d-%08x...\n",
					(*pList)->fig->space->dim,
					(*pList)->fig->index,
					(*pList)->fig->hash,
					fig->space->dim,
					fig->index,
					fig->hash);)
				if (convexFigListLen((*pList)->fig->parents)==1)
					convexFigDestroy((*pList)->fig);
				else {
					for (list2=(*pList)->fig->parents; list2; list2=list2->next)
						convexHullUtilAddInconsistent(inconsistent, list2->fig);
					convexFigBoundaryDetach(fig, (*pList)->fig);
				}
				removed++;
				break;
			}
			convexInteractUpdate();
		}
		convexInteractUpdate();
	}
	DEBUG_HULL_VERBOSE(printf("-FacesRm(%d) - exit\n", fig->space->dim);)
	return removed;
}

void convexHullUtilComplete(struct convexFig *fig, struct convexFigList **vertices, struct convexFigList **inconsistent) {
	// inconsistent are neighbours (same dim)
	// normals of faces needed
	DEBUG_HULL_VERBOSE(
		printf("-Complete(%d) - enter\n", fig->space->dim);
		DEBUG_HULL_DOT(convexFigPrint();))
	struct convexFig *face; // dim-1
	struct convexFig *edge; // dim-2
	struct convexFig *vert;
	struct convexFigList *list=0, *list2=0;
	GLfloat maxDist, dist;
	if (fig->boundary==0) {
		face=convexHullUtilInitialFace(*vertices, fig->space->dim-1, inconsistent);
		convexFigBoundaryAttach(fig, face);
		DEBUG_HULL(printf("Adding   %dD-%02d-%08x  to  %dD-%02d-%08x... (initial face)\n",
			face->space->dim,
			face->index,
			face->hash,
			fig->space->dim,
			fig->index,
			fig->hash);)
	}
	if (fig->space->dim==1) {
		if (fig->boundary->next) // at least two vertices bordering edge
			return;
		maxDist=-1;
		for (list=(*vertices); list; list=list->next) {
			dist=convexSpaceDist(fig->boundary->fig->space, list->fig->space);
			if (dist>maxDist) {
				vert=list->fig;
				maxDist=dist;
			}
		}
		convexFigBoundaryAttach(fig, vert);
	} else
		while (!convexInteractAborted) {
			convexFigMarkReset(convexFigMarkIdHull);
			convexFigGetLayer(fig, fig->space->dim-2, convexFigMarkIdTrue, convexFigMarkIdHull, &list);
			edge=0;
			for (; list; convexFigListRm(&list)) {
				if (convexFigGetLayer(list->fig, fig->space->dim-1, convexFigMarkIdHull, convexFigMarkIdTrue, &list2)==1) {
					edge=list->fig;  // edge which bounds just one face from fig
					face=list2->fig; // the face with opened boundary
					convexFigListDestroy(&list2);
					break;
				}
				convexFigListDestroy(&list2);
			}
			convexFigListDestroy(&list);
			if (!edge)
				break;
			DEBUG_HULL_VERBOSE(printf("-Complete: edge %8x selected\n", edge->hash);)

			for (list=(*vertices); list; list=list->next)
				if (convexSpaceContains(face->space, list->fig->space))
					convexFigListAdd(&list2, list->fig);
			if (convexHullUtilExpand(face, list2, inconsistent)) {
				convexFigListDestroy(&list2);
				continue;
			}
			convexFigListDestroy(&list2);

			convexSpaceCopy(edge->space, &tmpSpace);
			for (list=(*vertices); convexSpaceContains(face->space, list->fig->space); list=list->next);
			vert=list->fig;
			convexSpaceExpand(tmpSpace, vert->space);
			convexSpaceNormalCalc(tmpSpace, face->space);
			for (list=list->next; list; list=list->next) {
				if (convexSpaceContains(face->space, list->fig->space))
					continue;
				if (figureDistCmpZero(convexSpaceOrientedDist(tmpSpace, list->fig->space), figureDistCmpToleranceLower)>0) {
					vert=list->fig;
					convexSpaceReexpand(tmpSpace, vert->space);
					convexSpaceNormalCalc(tmpSpace, face->space);
				}
			}
			DEBUG_HULL_VERBOSE(printf("-Complete: vert %8x selected\n", vert->hash);)

			if (convexHullUtilExpandDim(edge, vert, &face, inconsistent)) {
				DEBUG_HULL_VERBOSE(printf("-Complete: space of face exists\n");)
				convexFigMarkReset(convexFigMarkIdHull);
				convexFigListMarkSet(*vertices, convexFigMarkIdHull);
				if (convexFigGetLayer(face, 0, convexFigMarkIdTrue, convexFigMarkIdHull, &list)) {
					DEBUG_HULL_VERBOSE(printf("-Complete: new vertices discovered\n");)
					convexHullUtilWrongFacesRm(fig, list, inconsistent);
					convexFigListMove(&list, vertices);
				}
			}
			convexSpaceNormalCalc(face->space, fig->space);
			convexFigBoundaryAttach(fig, face);

			DEBUG_HULL(printf("Adding   %dD-%02d-%08x  to  %dD-%02d-%08x... (%dD-%02d-%08x + %dD-%02d-%08x)\n",
				face->space->dim,
				face->index,
				face->hash,
				fig->space->dim,
				fig->index,
				fig->hash,
				edge->space->dim,
				edge->index,
				edge->hash,
				vert->space->dim,
				vert->index,
				vert->hash);)
			convexInteractUpdate();
		}

	fig->hash=0;
	convexFigMarkReset(convexFigMarkIdHash);
	convexFigHashCalc(fig, &fig->hash);
	DEBUG_HULL_VERBOSE(
		printf("-Complete(%d) - exit\n", fig->space->dim);
		DEBUG_HULL_DOT(convexFigPrint();))
}

struct convexFig *convexHullUtilInitialFace(struct convexFigList *vertices, int dim, struct convexFigList **inconsistent) {
	struct convexFig *fig;
	struct convexFig *vert=0;
	struct convexFigList *list=0;
	DEBUG_HULL_VERBOSE(printf("-InitialFace(%d) - enter\n", dim);)
	if (dim==0) {
		for (; vertices; vertices=vertices->next)
			if (!vert || convexSpaceCmpLex(vertices->fig->space, vert->space)<0)
				vert=vertices->fig;
		fig=vert;
	} else {
		fig=convexHullUtilInitialFace(vertices, dim-1, &list);
		for (; vertices; vertices=vertices->next)
			if ((!vert || (convexSpaceCmpLex(vertices->fig->space, vert->space)<0)) && (!convexSpaceContains(fig->space, vertices->fig->space)))
					vert=vertices->fig;
		if (vert) {
			convexHullUtilRepair(&list, inconsistent);
			convexHullUtilExpandDim(fig, vert, &fig, inconsistent);
		} else
			if (list)
				convexFigListMove(&list, inconsistent);
	}
	DEBUG_HULL_VERBOSE(printf("-InitialFace(%d) - exit\n", dim);)
	return fig;
}

struct convexFig *convexHullUtilCreate(struct convexFigList *vertices, struct convexFigList **inconsistent) {
	if (!vertices)
		return 0;
	struct convexFig *fig=convexHullUtilInitialFace(vertices, figureData.dim, inconsistent);
	convexHullUtilExpand(fig, vertices, inconsistent);
	return fig;
}

void convexHullUtilNormalsCalc(struct convexFig *fig) {
	struct convexFigList *list;
	for (list=fig->boundary; list; list=list->next)
		convexSpaceNormalCalc(list->fig->space, fig->space);
}

void convexHullUtilAddInconsistent(struct convexFigList **list, struct convexFig *fig) {
	if (!convexFigMarkGet(fig, convexFigMarkIdHullInconsistent)) {
		convexFigListAdd(list, fig);
		convexFigMarkSet(fig, convexFigMarkIdHullInconsistent);
	}
}
