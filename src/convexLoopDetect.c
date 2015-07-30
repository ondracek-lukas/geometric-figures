// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexLoopDetect.h"

#include "convexInteract.h"
#include "convexFig.h"
#include "debug.h"

static unsigned int selParent;
static unsigned int selChild;
static unsigned int count;
static unsigned int index;
static unsigned int next;
static unsigned int enabled=0;

void convexLoopDetectReset() {
	count=0;
	index=0;
	next=32;
	enabled=1;
}

void convexLoopDetectDisable() {
	enabled=0;
}

void convexLoopDetectAction(struct convexFig *parent, struct convexFig *child) {
	if (!enabled)
		return;
	if (++index==next) {
		selParent=parent->hash;
		selChild=child->hash;
		count=1;
		next*=2;
		DEBUG_LOOP(convexLoopDetectPrint();)
	} else
		if ((selParent==parent->hash) && (selChild==child->hash))
			if (++count>16) {
				convexInteractAbort("Error: loop detected, boundary update was aborted");
				convexLoopDetectDisable();
			}
}

