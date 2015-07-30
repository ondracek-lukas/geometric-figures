// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexFig.h"

#include <stdbool.h>

#include "convexSpace.h"

static unsigned int marksNumbers[convexFigMarkCount]= {1, 1, 1, 1};

static void hardReset(enum convexFigMarkId mark);

void convexFigMarkReset(enum convexFigMarkId mark) {
	if (mark<0)
		return;
	marksNumbers[mark]++;
	if (marksNumbers[mark]+1==0) {
		hardReset(mark);
		marksNumbers[mark]=1;
	}
}

static void hardReset(enum convexFigMarkId mark) {
	struct convexFigList *list=0;
	convexFigBstGetAll(convexSpaces, &list);
	while (list)
		convexFigMarkClear(convexFigListRm(&list), mark);
}

void convexFigMarkSet(struct convexFig *fig, enum convexFigMarkId mark) {
	if (mark<0)
		return;
	fig->mark[mark]=marksNumbers[mark];
}

void convexFigMarkClear(struct convexFig *fig, enum convexFigMarkId mark) {
	if (mark<0)
		return;
	fig->mark[mark]=0;
}

bool convexFigMarkGet(struct convexFig *fig, enum convexFigMarkId mark) {
	if (mark==convexFigMarkIdTrue)
		return 1;
	return (fig->mark[mark]==marksNumbers[mark]);
}
