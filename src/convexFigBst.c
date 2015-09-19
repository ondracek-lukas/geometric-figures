// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexFig.h"

#include <math.h>

#include "convex.h"
#include "figure.h"
#include "convexSpace.h"
#include "safe.h"
#include "debug.h"

struct convexFigBst {
	struct convexFig *fig;
	struct convexFigBst *left;
	struct convexFigBst *right;
	int leaning;
};

static struct convexFigBst *freed=0;

static int removeMin(struct convexFigBst **bst, struct convexFig **min);
static struct convexFigBst *newBst(struct convexFig *fig);
static void deleteBst(struct convexFigBst *bst);
static int rotate(struct convexFigBst **root);

int convexFigBstAdd(struct convexFigBst **bst, struct convexFig *fig) {
	int depthDiff;
	if (!*bst) {
		*bst=newBst(fig);
		return 1;
	}

	if (fig->space->hash < (*bst)->fig->space->hash)
		(*bst)->leaning+=depthDiff=-convexFigBstAdd(&(*bst)->left, fig);
	else
		(*bst)->leaning+=depthDiff=convexFigBstAdd(&(*bst)->right, fig);

	depthDiff=depthDiff*(*bst)->leaning>0;
	if (abs((*bst)->leaning)>1)
		return depthDiff+rotate(bst);
	else
		return depthDiff;
}

int convexFigBstRm(struct convexFigBst **bst, struct convexFig *fig) {
	int depthDiff;
	struct convexFigBst *bst2;
	if (!*bst) {
		return 0;
	}

	if ((*bst)->fig==fig) {
		if ((*bst)->right) {
			(*bst)->leaning+=depthDiff=removeMin(&(*bst)->right, &fig);
			(*bst)->fig=fig;
		} else {
			bst2=(*bst)->left;
			deleteBst(*bst);
			*bst=bst2;
			return -1;
		}
	} else {
		depthDiff=0;
		if (fig->space->hash <= (*bst)->fig->space->hash)
			(*bst)->leaning+=depthDiff=-convexFigBstRm(&(*bst)->left, fig);
		if ((!depthDiff) && (fig->space->hash >= (*bst)->fig->space->hash))
			(*bst)->leaning+=depthDiff=convexFigBstRm(&(*bst)->right, fig);
	}

	depthDiff=-(depthDiff && (depthDiff*(*bst)->leaning<=0));
	if (abs((*bst)->leaning)>1)
		return depthDiff+rotate(bst);
	else
		return depthDiff;
}

int removeMin(struct convexFigBst **bst, struct convexFig **min) {
	int depthDiff;
	struct convexFigBst *bst2;
	if (!(*bst)->left) {
		*min=(*bst)->fig;
		bst2=(*bst)->right;
		deleteBst(*bst);
		*bst=bst2;
		return -1;
	}

	(*bst)->leaning+=depthDiff=-removeMin(&(*bst)->left, min);
	depthDiff=-(depthDiff && ((*bst)->leaning<=0));
	if (abs((*bst)->leaning)>1)
		return depthDiff+rotate(bst);
	else
		return depthDiff;
}

int convexFigBstFind(struct convexFigBst *bst, struct convexSpace *space, struct convexFigList **list) {
	int cmp, count=0;
	if (!bst)
		return 0;

	cmp=figureDistCmp(space->hash, bst->fig->space->hash, figureDistCmpToleranceHigher);
	if (cmp>=0)
		count+=convexFigBstFind(bst->right, space, list);
	if ((cmp==0) && (convexSpaceEq(space, bst->fig->space))) {
		count++;
		convexFigListAdd(list, bst->fig);
	}
	if (cmp<=0)
		count+=convexFigBstFind(bst->left, space, list);
	return count;
}

int convexFigBstGetAll(struct convexFigBst *bst, struct convexFigList **list) {
	if (!bst)
		return 0;
	int count=1;
	count+=convexFigBstGetAll(bst->right, list);
	convexFigListAdd(list, bst->fig);
	count+=convexFigBstGetAll(bst->left, list);
	return count;
}

struct convexFigBst *newBst(struct convexFig *fig) {
	struct convexFigBst *bst;
	if (freed) {
		bst=freed;
		freed=bst->right;
	} else
		bst=safeMalloc(sizeof(struct convexFigBst));
	bst->left=0;
	bst->right=0;
	bst->leaning=0;
	bst->fig=fig;
	return bst;
}

void deleteBst(struct convexFigBst *bst) {
	DEBUG_HULL(
		if (bst->leaning==-100) {
			printf("err: bst deleted twice\n");
			exit(4);
		}
		bst->leaning=-100;
	)

	bst->right=freed;
	freed=bst;
}

int rotate(struct convexFigBst **root) {
	struct convexFigBst **deep, **middle, *tmp;

	if ((*root)->leaning<0) // left to right
		deep=&(*root)->left;
	else                   // right to left
		deep=&(*root)->right;

	if ((*deep)->leaning*(*root)->leaning==-2)
		rotate(deep);

	if ((*root)->leaning<0)
		middle=&(*deep)->right;
	else
		middle=&(*deep)->left;

	int t=((*root)->leaning>0?1:-1);
	(*root)->leaning+=-t-(*deep)->leaning*(t*(*deep)->leaning>0);
	(*deep)->leaning+=-t+(*root)->leaning*(t*(*root)->leaning<0);

	tmp=*root;
	*root=*deep;
	*deep=*middle;
	*middle=tmp;

	return ((*root)->leaning?0:-1);
}
