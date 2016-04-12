// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexFig.h"

#include <stdbool.h>

#include "convexSpace.h"

static unsigned int marksVersions[convexFigMarkCount]= {1, 1, 1, 1};

unsigned int convexFigMarkReset(enum convexFigMarkId mark) {
	return ++marksVersions[mark];
}

static void hardResetR(struct convexFigList *figures);

extern void convexFigMarkResetHard(struct convexFigList *allFigures) {
	convexFigMarkReset(0);
	for (struct convexFigList *list=allFigures; list; list=list->next) {
		convexFigGetLayer(list->fig, 0, convexFigMarkIdTrue, 0, 0); // mark all with mark 0
	}
	hardResetR(allFigures);
	for (int mark=0; mark<convexFigMarkCount; mark++) {
		marksVersions[mark]=1;
	}
}

static void hardResetR(struct convexFigList *figures) {
	for (; figures; figures=figures->next) {
		if (convexFigMarkGet(figures->fig, 0)) {
			hardResetR(figures->fig->boundary);
			for (int mark=0; mark<convexFigMarkCount; mark++) {
				figures->fig->mark[mark]=0;
			}
		}
	}
}

void convexFigMarkSet(struct convexFig *fig, enum convexFigMarkId mark) {
	if (mark<0)
		return;
	fig->mark[mark]=marksVersions[mark];
}
void convexFigMarkSetV(struct convexFig *fig, enum convexFigMarkId mark, unsigned int version) {
	fig->mark[mark]=version;
}

void convexFigMarkClear(struct convexFig *fig, enum convexFigMarkId mark) {
	fig->mark[mark]=0;
}

bool convexFigMarkGet(struct convexFig *fig, enum convexFigMarkId mark) {
	if (mark==convexFigMarkIdTrue)
		return 1;
	return (fig->mark[mark]==marksVersions[mark]);
}
bool convexFigMarkGetV(struct convexFig *fig, enum convexFigMarkId mark, unsigned int version) {
	return (fig->mark[mark]==version);
}

bool convexFigMarkNegGet(struct convexFig *fig, enum convexFigMarkId mark) {
	if (fig->mark[mark]==marksVersions[mark]) {
		fig->mark[mark]--;
		return 0;
	} else {
		fig->mark[mark]=marksVersions[mark];
		return 1;
	}
}
bool convexFigMarkNegGetV(struct convexFig *fig, enum convexFigMarkId mark, unsigned int version) {
	if (fig->mark[mark]==version) {
		fig->mark[mark]=0;
		return 0;
	} else {
		fig->mark[mark]=version;
		return 1;
	}
}
