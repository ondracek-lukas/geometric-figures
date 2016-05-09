// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexHullUtil.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "convex.h"
#include "convexSpace.h"
#include "convexFig.h"
#include "convexInteract.h"
#include "figure.h"
#include "debug.h"
#include "matrix.h"

struct convexFig *convexHullUtilExpandDim(struct convexFig *fig, struct convexFigList *newVertices, struct convexSpace *newSpace) {
	unsigned int hash=0;
	int vertCount;
	struct convexFigList *list=0;
	struct convexFig *newFig;
	DEBUG_HULL_VERBOSE(printf("-ExpandDim(%d) - enter\n", fig->space->dim+1);)

	convexFigMarkReset(convexFigMarkIdHash);
	vertCount=convexFigHashCalc(fig, &hash);
	for (list=newVertices; list; list=list->next) {
		if (list->fig->parents) { // if figure exists, bordering vertices == vertices with parents
			vertCount+=convexFigHashCalc(list->fig, &hash);
		}
	}
	if ((newFig=convexFigHashFind(hash, newSpace->dim, vertCount))) {
		DEBUG_HULL_VERBOSE(printf("-ExpandDim(%d) - exit (fig exists)\n", fig->space->dim+1);)
		return newFig;
	}

	newFig=convexFigNew();
	convexSpaceAssign(newSpace, newFig);
	convexFigBoundaryAttach(newFig, fig);
	struct convexFigList *allVertices=0;
	convexFigMarkReset(convexFigMarkIdHull);
	convexFigGetLayer(fig, 0, convexFigMarkIdTrue, convexFigMarkIdHull, &allVertices);
	convexFigListCopy(newVertices, &allVertices, convexFigMarkIdHull);
	convexSpaceCenterPos(newFig->space, allVertices);
	convexSpaceNormalCalc(fig->space, newSpace);

	convexHullUtilComplete(newFig, allVertices);
	convexFigMarkSet(newFig, convexFigMarkIdHullProcessed);

	DEBUG_HULL_VERBOSE(printf("-ExpandDim(%d) - exit (created)\n", fig->space->dim+1);)
	return newFig;
}


void convexHullUtilComplete(struct convexFig *fig, struct convexFigList *vertices) {
	// normals of facets needed
	DEBUG_HULL_VERBOSE(
		printf("-Complete(%d) - enter\n", fig->space->dim);
		DEBUG_HULL_DOT(convexFigPrint();))

	if (fig->space->dim==1) {
		if (fig->boundary->next) // at least two vertices bordering edge
			return;
		GLdouble maxDist, dist;
		maxDist=-1;
		struct convexFig *vert;
		for (struct convexFigList *vertList=vertices; vertList; vertList=vertList->next) {
			dist=convexSpaceDist(fig->boundary->fig->space, vertList->fig->space);
			if (dist>maxDist) {
				vert=vertList->fig;
				maxDist=dist;
			}
		}
		convexFigBoundaryAttach(fig, vert);
	} else {
		// Mark ridges with just one parent within facets of the figure
		// older oneParent marks in higner dimensions won't be change
		// using newer oneParent marks in lower dimensions is allowed
		unsigned int oneParentMarkV=convexFigMarkReset(convexFigMarkIdHullOneParent);
		unsigned int moreParentsMarkV=convexFigMarkReset(convexFigMarkIdHullOneParent);
		for (struct convexFigList *facets=fig->boundary; facets; facets=facets->next) {
			for (struct convexFigList *ridges=facets->fig->boundary; ridges; ridges=ridges->next) {
				if (convexFigMarkGetV(ridges->fig, convexFigMarkIdHullOneParent, moreParentsMarkV)) {
					convexInteractAbort("Error: more than two facets sharing ridge");
				} else if(convexFigMarkGetV(ridges->fig, convexFigMarkIdHullOneParent, oneParentMarkV)) {
					convexFigMarkSetV(ridges->fig, convexFigMarkIdHullOneParent, moreParentsMarkV);
				} else {
					convexFigMarkSetV(ridges->fig, convexFigMarkIdHullOneParent, oneParentMarkV);
				}
			}
		}

		/* Update facets near holes (and oneParent marks in ridges if needed)
		 * Needed only in case of update
		for (struct convexFigList *facets=fig->boundary; facets; facets=facets->next) {
			struct convexFig *facet=facets->fig;
			if (!convexFigMarkGet(facet, convexFigMarkIdHullProcessed)) {
				bool holeNeighbour=false;
				for (struct convexFigList *ridges=facet->boundary; ridges; ridges=ridges->next) {
					if (convexFigMarkGetV(ridges->fig, convexFigMarkIdHullOneParent, oneParentMarkV)) {
						holeNeighbour=true;
						break;
					}
				}
				if (holeNeighbour) {
					for (struct convexFigList *ridges=facet->boundary; ridges; ridges=ridges->next) {
						convexFigMarkNegGetV(ridges->fig, convexFigMarkIdHullOneParent, oneParentMarkV);
					}
					struct convexFigList *facetVertices=convexHullUtilSpaceFilter(vertices, facet->space);
					convexHullUtilExpand(facet, facetVertices);
					convexFigListDestroy(&facetVertices);
					for (struct convexFigList *ridges=facet->boundary; ridges; ridges=ridges->next) {
						convexFigMarkNegGetV(ridges->fig, convexFigMarkIdHullOneParent, oneParentMarkV);
					}
				}
			}
		}
		*/


		// Create missing facets, begin from hole-bordering ridges
		struct convexFigList *facets=0;
		convexFigListCopy(fig->boundary, &facets, convexFigMarkIdTrue);
		while (facets && !convexInteractAborted) {
			struct convexFig *facet=convexFigListRm(&facets);
			for (struct convexFigList *ridges=facet->boundary; ridges && !convexInteractAborted; ridges=ridges->next) {
				struct convexFig *ridge=ridges->fig;
				if (!convexFigMarkGetV(ridge, convexFigMarkIdHullOneParent, oneParentMarkV)) {
					continue;
				}

				DEBUG_HULL_VERBOSE(printf("-Complete: ridge %8x selected\n", ridge->hash);)

				struct convexSpace *facetSpace=0;
				convexSpaceCopy(ridge->space, &facetSpace);
				struct convexFigList *vertList=vertices;

				for(; convexSpaceContains(facet->space, vertList->fig->space); vertList=vertList->next);
				struct convexFig *vert=vertList->fig;
				struct convexFigList *facetVertices=0;
				convexFigListAdd(&facetVertices, vert);
				convexSpaceExpand(facetSpace, vert->space);
				convexSpaceNormalCalc(facetSpace, facet->space);
				for (vertList=vertList->next; vertList; vertList=vertList->next) {
					if (convexSpaceContains(facet->space, vertList->fig->space))
						continue;
					int cmp=figureDistCmpZero(convexSpaceOrientedDist(facetSpace, vertList->fig->space));
					if (cmp>0) {
						vert=vertList->fig;
						convexSpaceReexpand(facetSpace, vert->space);
						convexSpaceNormalCalc(facetSpace, facet->space);
						convexFigListAdd(&facetVertices, vertList->fig);
					} else if (cmp==0) {
						convexFigListAdd(&facetVertices, vertList->fig);
					}
				}
				for (struct convexFigList **pList=&facetVertices; *pList; ) {
					if (!convexSpaceContains(facetSpace, (*pList)->fig->space)) {
						convexFigListRm(pList);
					} else {
						pList=&(*pList)->next;
					}
				}

				DEBUG_HULL_VERBOSE(printf("-Complete: vert %8x selected\n", vert->hash);)

				struct convexFig *newFacet=convexHullUtilExpandDim(ridge, facetVertices, facetSpace);
				convexSpaceNormalCalc(newFacet->space, fig->space);
				convexFigBoundaryAttach(fig, newFacet);

				convexFigListAdd(&facets, newFacet);
				for (struct convexFigList *ridges2=newFacet->boundary; ridges2; ridges2=ridges2->next) {
					if (convexFigMarkGetV(ridges2->fig, convexFigMarkIdHullOneParent, moreParentsMarkV)) {
						convexInteractAbort("Error: more than two facets sharing ridge");
					} else if(convexFigMarkGetV(ridges2->fig, convexFigMarkIdHullOneParent, oneParentMarkV)) {
						convexFigMarkSetV(ridges2->fig, convexFigMarkIdHullOneParent, moreParentsMarkV);
					} else {
						convexFigMarkSetV(ridges2->fig, convexFigMarkIdHullOneParent, oneParentMarkV);
					}
				}

				DEBUG_HULL(printf("Adding   %dD-%02d-%08x  to  %dD-%02d-%08x... (%dD-%02d-%08x + %dD-%02d-%08x)\n",
					facet->space->dim,
					facet->index,
					facet->hash,
					fig->space->dim,
					fig->index,
					fig->hash,
					ridge->space->dim,
					ridge->index,
					ridge->hash,
					vert->space->dim,
					vert->index,
					vert->hash);)
				convexInteractUpdate();
			}
		}
		convexFigListDestroy(&facets);
	}

	DEBUG_HULL_PROGR(unsigned int oldHash=fig->hash;)
	fig->hash=0;
	convexFigListDestroy(&fig->vertices);
	for (struct convexFigList *vertList=vertices; vertList; vertList=vertList->next) {
		if (vertList->fig->parents) {
			convexFigListAdd(&fig->vertices, vertList->fig);
		}
	}
	convexFigMarkReset(convexFigMarkIdHash);
	convexFigHashCalc(fig, &fig->hash);
	convexFigHashAdd(fig);
	DEBUG_HULL_PROGR(debugProgrHashChange(oldHash, fig->hash);)
	DEBUG_HULL_VERBOSE(
		printf("-Complete(%d) - exit\n", fig->space->dim);
		DEBUG_HULL_DOT(convexFigPrint();))
}

struct convexFig *convexHullUtilCreate(struct convexFigList *vertices) {
	if (!vertices) {
		return 0;
	}

	struct convexSpace *inSpace=0;
	convexSpaceCopy(vertices->fig->space, &inSpace);
	for (struct convexFigList *list=vertices->next; list; list=list->next) {
		if (!convexSpaceContains(inSpace, list->fig->space)) {
			convexSpaceExpand(inSpace, list->fig->space);
		}
	}

	if (inSpace->dim==0) {
		return vertices->fig;
	}

	struct convexSpace *hyppSpace=0;
	convexSpaceCopy(inSpace, &hyppSpace);
	convexSpaceDecreaseDim(hyppSpace);

	struct convexFig *minVert=vertices->fig;
	GLdouble min=convexSpaceOrientedDist(hyppSpace, minVert->space);
	for (struct convexFigList *list=vertices->next; list; list=list->next) {
		GLdouble d=convexSpaceOrientedDist(hyppSpace, list->fig->space);
		if (d<min) {
			min=d;
			minVert=list->fig;
		}
	}

	convexSpaceMoveTo(hyppSpace, minVert);

	struct convexFigList *vertices2 = convexHullUtilSpaceFilter(vertices, hyppSpace);
	struct convexFig *fig = convexHullUtilCreate(vertices2);
	convexFigListDestroy(&vertices2);

	while (hyppSpace->dim > fig->space->dim) {
		struct convexSpace *figSpace=0;
		convexSpaceCopy(fig->space, &figSpace);

		min=1.1;
		minVert=0;
		for (struct convexFigList *list=vertices; list; list=list->next) {
			if (!convexSpaceContains(figSpace, list->fig->space)) {
				convexSpaceExpand(figSpace, list->fig->space);
				convexSpaceDecreaseDim(figSpace);
				GLdouble cosine=fabs(matrixScalarProduct(hyppSpace->normal, figSpace->normal, hyppSpace->coordsCnt));
				if (min>cosine) {
					min=cosine;
					minVert=list->fig;
				}
			}
		}
		convexSpaceExpand(figSpace, minVert->space);

		vertices2=convexHullUtilSpaceFilter(vertices, figSpace);
		fig=convexHullUtilExpandDim(fig, vertices2, figSpace);
		convexFigListDestroy(&vertices2);
	}

	if (fig->space->dim == hyppSpace->dim) {
		fig=convexHullUtilExpandDim(fig, vertices, inSpace);
	} else {
		convexInteractAbort("Error: generated figure has wrong dimension");
	}

	convexSpaceDestroy(&inSpace);
	convexSpaceDestroy(&hyppSpace);
	return fig;
}

struct convexFigList *convexHullUtilSpaceFilter(struct convexFigList *figures, struct convexSpace *inSpace) {
	struct convexFigList *filtered=0;
	for (; figures; figures=figures->next) {
		if (convexSpaceContains(inSpace, figures->fig->space)) {
			convexFigListAdd(&filtered, figures->fig);
		}
	}
	return filtered;
}

void convexHullUtilNormalsCalc(struct convexFig *fig) {
	struct convexFigList *list;
	for (list=fig->boundary; list; list=list->next)
		convexSpaceNormalCalc(list->fig->space, fig->space);
}

