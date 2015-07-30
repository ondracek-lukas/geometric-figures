// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexSpace.h"

#include <math.h>

#include "figure.h"
#include "safe.h"
#include "matrix.h"
#include "convexFig.h"

struct convexFigBst *convexSpaces=0;
// struct convexSpace *convexSpace=0; // !! temp

static int dim=-1;
static struct convexSpace *hashPoint=0;
static GLfloat *tempVect=0;

static void hashCalc(struct convexSpace *space);

static void init() {
	int i;
	if (dim!=figureData.dim) {
		dim=figureData.dim;
		if (!hashPoint)
			hashPoint=safeMalloc(sizeof(struct convexSpace));
		else
			free(hashPoint->pos);
		free(tempVect);

		tempVect=safeMalloc(dim*sizeof(GLfloat));
		hashPoint->pos=safeMalloc(dim*sizeof(GLfloat));
		for (i=0; i<dim; i++)
			hashPoint->pos[i]=rand()-RAND_MAX/2;
		matrixScale(
			hashPoint->pos,
			1.0/matrixVectorNorm(hashPoint->pos, dim)*rand()/RAND_MAX,
			dim);
	}
}

void create(struct convexSpace **pSpace) {
	init();
	if (*pSpace && ((*pSpace)->coordsCnt==dim))
		return;

	if (*pSpace) {
		free((*pSpace)->pos);
		free((*pSpace)->ortBasis);
		free((*pSpace)->normal);
	} else {
		*pSpace=safeMalloc(sizeof(struct convexSpace));
	}
	(*pSpace)->coordsCnt=dim;
	(*pSpace)->pos=safeMalloc(dim*sizeof(GLfloat));
	(*pSpace)->ortBasis=safeMalloc(dim*dim*sizeof(GLfloat));
	(*pSpace)->normal=safeMalloc(dim*sizeof(GLfloat));
}

void convexSpaceCreateVert(struct convexSpace **pSpace, GLfloat *pos) {
	create(pSpace);
	(*pSpace)->dim=0;
	matrixCopy(pos, (*pSpace)->pos, dim);
	hashCalc(*pSpace);
}

void convexSpaceCreate(struct convexSpace **pSpace, struct convexFig *fig) {
	struct convexFigList *vertices=0;
	create(pSpace);
	convexFigGetLayer(fig, 0, convexFigMarkIdTrue, convexFigMarkIdTrue, &vertices);
	convexSpaceCenterPos(*pSpace, vertices);
	(*pSpace)->dim=0;
	while (vertices && ((*pSpace)->dim<dim)) {
		matrixCopy(
			convexFigListRm(&vertices)->space->pos,
			(*pSpace)->ortBasis+(*pSpace)->dim*dim,
			dim);
		matrixAddScaled(
			(*pSpace)->ortBasis+(*pSpace)->dim*dim,
			-1, (*pSpace)->pos,
			dim);
		matrixOrtVectorToBasis(
			(*pSpace)->ortBasis,
			(*pSpace)->ortBasis+(*pSpace)->dim*dim,
			(*pSpace)->dim, dim);
		if (figureDistCmpZeroSq(matrixVectorNormSq((*pSpace)->ortBasis+(*pSpace)->dim*dim, dim), figureDistCmpToleranceDefault)!=0) {
			matrixVectorNormalize((*pSpace)->ortBasis+(*pSpace)->dim*dim, dim);
			(*pSpace)->dim++;
		}
	}
	hashCalc(*pSpace);
}

void convexSpaceDestroy(struct convexSpace **pSpace) {
	free((*pSpace)->pos);
	free((*pSpace)->ortBasis);
	free((*pSpace)->normal);
	free(*pSpace);
	*pSpace=0;
}


void convexSpaceCopy(struct convexSpace *src, struct convexSpace **pDst) {
	create(pDst);
	(*pDst)->dim=src->dim;
	matrixCopy(src->pos, (*pDst)->pos, dim);
	matrixCopy(src->ortBasis, (*pDst)->ortBasis, dim*(*pDst)->dim);
	(*pDst)->hash=src->hash;
}


void convexSpaceExpand(struct convexSpace *space, struct convexSpace *vert) {
	matrixCopy(
		vert->pos,
		space->ortBasis+space->dim*dim,
		dim);
	matrixAddScaled(
		space->ortBasis+space->dim*dim,
		-1, space->pos,
		dim);
	matrixOrtVectorToBasis(
		space->ortBasis,
		space->ortBasis+space->dim*dim,
		space->dim, dim);
	matrixVectorNormalize(
		space->ortBasis+space->dim*dim,
		dim);
	space->dim++;
	hashCalc(space);
}

void convexSpaceReexpand(struct convexSpace *space, struct convexSpace *vert) {
	space->dim--;
	convexSpaceExpand(space, vert);
}

void convexSpaceAssign(struct convexSpace *space, struct convexFig *fig) {
	fig->space->dim=space->dim;
	matrixCopy(space->pos, fig->space->pos, dim);
	fig->space->ortBasis=safeMalloc(space->dim*dim*sizeof(GLfloat));
	matrixCopy(space->ortBasis, fig->space->ortBasis, dim*space->dim);
	fig->space->hash=space->hash;
	convexFigBstAdd(&convexSpaces, fig);
}

void convexSpaceUnassign(struct convexFig *fig) {
	convexFigBstRm(&convexSpaces, fig);
	free(fig->space->ortBasis);
	fig->space->ortBasis=0;
}

void convexSpaceCenterPos(struct convexSpace *space, struct convexFigList *vertices) {
	int vertCount=0;
	matrixZero(space->pos, dim);
	if (!vertices)
		return;
	while (vertices) {
		matrixAdd(space->pos, vertices->fig->space->pos, dim);
		vertCount++;
		vertices=vertices->next;
	}
	matrixScale(space->pos, 1.0/vertCount, dim);
}

int convexSpaceGetFigs(struct convexSpace *space, struct convexFigList **list) {
	return convexFigBstFind(convexSpaces, space, list);
}

void convexSpaceNormalCalc(struct convexSpace *space, struct convexSpace *inSpace) {
	int i;
	for (i=inSpace->dim-1; i>=0; i--) {
		matrixCopy(
			inSpace->ortBasis+i*dim,
			space->normal,
			dim);
		matrixOrtVectorToBasis(
			space->ortBasis,
			space->normal,
			space->dim, dim);
		if (figureDistCmpZeroSq(matrixVectorNormSq(space->normal, dim), figureDistCmpToleranceDefault)!=0) {
			matrixVectorNormalize(space->normal, dim);
			break;
		}
	}
	space->normalPos=0;
	space->normalPos+=convexSpaceOrientedDist(space, space);
	if (convexSpaceOrientedDist(space, inSpace)>0) {
		matrixScale(space->normal, -1, dim);
		space->normalPos*=-1;
	}
}

void hashCalc(struct convexSpace *space) {
	space->hash=convexSpaceDist(space, hashPoint);
}

bool convexSpaceEq(struct convexSpace *space1, struct convexSpace *space2) {
	return (space1->dim==space2->dim) && convexSpaceContains(space1, space2);
}

bool convexSpaceContains(struct convexSpace *space1, struct convexSpace *space2) {
	int i;
	if (space1->dim<space2->dim)
		return 0;
	if (figureDistCmpZeroSq(convexSpaceDistSq(space1, space2), figureDistCmpToleranceDefault)!=0)
		return 0;
	for (i=0; i<space2->dim; i++) {
		matrixCopy(
			space2->ortBasis+i*dim,
			tempVect,
			dim);
		matrixOrtVectorToBasis(
			space1->ortBasis,
			tempVect,
			space1->dim, dim);
		if (figureDistCmpZeroSq(matrixVectorNormSq(tempVect, dim), figureDistCmpToleranceDefault)!=0)
			return 0;
	}
	return 1;
}

GLfloat convexSpaceDist(struct convexSpace *space, struct convexSpace *vert) {
	return sqrt(convexSpaceDistSq(space, vert));
}

GLfloat convexSpaceDistSq(struct convexSpace *space, struct convexSpace *vert) {
	matrixCopy(vert->pos, tempVect, dim);
	matrixAddScaled(tempVect, -1, space->pos, dim);
	matrixOrtVectorToBasis(space->ortBasis, tempVect, space->dim, dim);
	return matrixVectorNormSq(tempVect, dim);
}

GLfloat convexSpaceOrientedDist(struct convexSpace *space, struct convexSpace *vert) {
	return matrixScalarProduct(space->normal, vert->pos, dim)-space->normalPos;
}

int convexSpaceCmpLex(struct convexSpace *vert1, struct convexSpace *vert2) {
	int i, cmp;
	for (i=0; i<dim; i++)
		if ((cmp=figureDistCmp(vert1->pos[i], vert2->pos[i], figureDistCmpToleranceDefault))!=0)
			return cmp;
	return 0;
}
