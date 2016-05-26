// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmd.h"

#include <stdlib.h>

#include "figure.h"
#include "console.h"
#include "matrix.h"
#include "safe.h"
#include "script.h"
#include "drawer.h"

int consoleCmdVertexSelected=-1;

void consoleCmdVertexSelect(int index) {
	if ((figureData.dim>=0) && (index>=0) && (index<figureData.count[0])) {
		consoleCmdVertexSelected=index;
		drawerInvokeRedisplay();
	} else
		scriptThrowException("Wrong index");
}

int consoleCmdVertexGetSelected() {
	return consoleCmdVertexSelected;
}

void consoleCmdVertexNext() {
	if ((figureData.dim>=0) && (figureData.count[0])) {
		consoleCmdVertexSelected=(consoleCmdVertexSelected+1)%figureData.count[0];
		drawerInvokeRedisplay();
	} else
		scriptThrowException("No vertex exist");
}

void consoleCmdVertexPrevious() {
	if ((figureData.dim>=0) && (figureData.count[0])) {
		if (consoleCmdVertexSelected==-1)
			consoleCmdVertexSelected=figureData.count[0]-1;
		else
			consoleCmdVertexSelected=(consoleCmdVertexSelected+figureData.count[0]-1)%figureData.count[0];
		drawerInvokeRedisplay();
	} else
		scriptThrowException("No vertex exist");
}

void consoleCmdVertexDeselect() {
	consoleCmdVertexSelected=-1;
	drawerInvokeRedisplay();
}
