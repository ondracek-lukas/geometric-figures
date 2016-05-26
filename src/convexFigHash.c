// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include <stdlib.h>
#include "convexFig.h"
#include "convexSpace.h"


struct convexFigList **hashTbl=0;
unsigned hashTblSize=0;
unsigned hashTblCnt=0;
#define HASH_TBL(hash) hashTbl[hash % hashTblSize]

static void hashTblResize(int newSize) {
	struct convexFigList **oldTbl=hashTbl;
	int oldSize=hashTblSize;
	hashTbl=calloc(newSize, sizeof(struct convexFigList *));
	hashTblSize=newSize;
	for (int i=0; i<oldSize; i++) {
		while (oldTbl[i]) {
			struct convexFig *fig=convexFigListRm(&oldTbl[i]);
			convexFigListAdd(&HASH_TBL(fig->hash), fig);
		}
	}
	free(oldTbl);
}


void convexFigHashAdd(struct convexFig *fig) {
	hashTblCnt++;
	if (hashTblCnt*2>hashTblSize) { // has to hold at first time
		unsigned int newSize=128;
		while (hashTblCnt*2>newSize) {
			newSize<<=1;
		}
		hashTblResize(newSize);
	}
	convexFigListAdd(&HASH_TBL(fig->hash), fig);
}

void convexFigHashRm(struct convexFig *fig) {
	if (hashTblSize==0) {
		return;
	}
	struct convexFigList **pList=&HASH_TBL(fig->hash);
	while (*pList) {
		if ((*pList)->fig==fig) {
			convexFigListRm(pList);
			hashTblCnt--;
		} else {
			pList=&(*pList)->next;
		}
	}
}

int convexFigHashCalc(struct convexFig *fig, unsigned int *hash) {
	// hash-marked vertices are skipped, the others are marked
	if (fig->space->dim==0) {
		if (!convexFigMarkGet(fig, convexFigMarkIdHash)) {
			convexFigMarkSet(fig, convexFigMarkIdHash);
			*hash^=fig->hash;
			return 1;
		} else {
			return 0;
		}
	} else {
		int vertCount=0;
		for (struct convexFigList *vertices=fig->vertices; vertices; vertices=vertices->next) {
			if (!convexFigMarkGet(vertices->fig, convexFigMarkIdHash)) {
				convexFigMarkSet(vertices->fig, convexFigMarkIdHash);
				*hash^=vertices->fig->hash;
				vertCount++;
			}
		}
		return vertCount;
	}
}

static int hashFigEqual(struct convexFig *fig) {
	// all vertices hash-marked ? returns count : returns 0
	if (fig->space->dim==0) {
		return convexFigMarkGet(fig, convexFigMarkIdHash);
	} else {
		int vertCount=0;
		for (struct convexFigList *vertices=fig->vertices; vertices; vertices=vertices->next) {
			if (!convexFigMarkGet(vertices->fig, convexFigMarkIdHash)) {
				return 0;
			}
			vertCount++;
		}
		return vertCount;
	}
}

struct convexFig *convexFigHashFind(unsigned int hash, int dim, int vertCount) {
	// returns figure from list with given hash and all vertices (given count) hash-marked or 0
	if (hashTblSize==0) {
		return 0;
	}
	struct convexFigList *list=HASH_TBL(hash);
	while (list) {
		if ((list->fig->hash==hash) && (list->fig->space->dim==dim) && (hashFigEqual(list->fig)==vertCount)) {
			return list->fig;
		}
		list=list->next;
	}
	return 0;
}
