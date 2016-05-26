// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexFig.h"

#include "safe.h"
struct convexFigList *freed=0;

void convexFigListAdd(struct convexFigList **pList, struct convexFig *fig) {
	struct convexFigList *list2;
	if (freed) {
		list2=freed;
		freed=list2->next;
	} else
		list2=safeMalloc(sizeof(struct convexFigList));
	list2->fig=fig;
	list2->next=(*pList);
	(*pList)=list2;
}

struct convexFig *convexFigListRm(struct convexFigList **pList) {
	struct convexFigList *list2=(*pList);
	(*pList)=(*pList)->next;
	list2->next=freed;
	freed=list2;
	return list2->fig;
}

int convexFigListRmFig(struct convexFigList **pList, struct convexFig *fig) {
	int count=0;
	while (*pList)
		if ((*pList)->fig==fig) {
			convexFigListRm(pList);
			count++;
		} else
			pList=&(*pList)->next;
	return count;
}

int convexFigListContains(struct convexFigList *list, struct convexFig *fig) {
	int count=0;
	for (; list; list=list->next)
		if (list->fig==fig)
			count++;
	return count;
}

void convexFigListDestroy(struct convexFigList **pList) {
	while (*pList)
		convexFigListRm(pList);
}

int convexFigListLen(struct convexFigList *list) {
	int i=0;
	while (list) {
		i++;
		list=list->next;
	}
	return i;
}

void convexFigListCopy(struct convexFigList *src, struct convexFigList **dst, enum convexFigMarkId mark) {
	while (src) {
		if ((mark==convexFigMarkIdTrue) || !convexFigMarkGet(src->fig, mark)) {
			convexFigMarkSet(src->fig, mark);
			convexFigListAdd(dst, src->fig);
			dst=&(*dst)->next;
		}
		src=src->next;
	}
}

void convexFigListMove(struct convexFigList **src, struct convexFigList **dst) {
	while (*src) {
		convexFigListAdd(dst, convexFigListRm(src));
		dst=&(*dst)->next;
	}
}

void convexFigListMarkSet(struct convexFigList *list, enum convexFigMarkId mark) {
	while (list) {
		convexFigMarkSet(list->fig, mark);
		list=list->next;
	}
}

void convexFigListMarkClear(struct convexFigList *list, enum convexFigMarkId mark) {
	while (list) {
		convexFigMarkClear(list->fig, mark);
		list=list->next;
	}
}
