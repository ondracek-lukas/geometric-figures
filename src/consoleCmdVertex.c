// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmd.h"

#include <stdlib.h>

#include "figure.h"
#include "console.h"
#include "matrix.h"
#include "safe.h"

int consoleCmdVertexSelected=-1;

void consoleCmdVertexSelect(int index) {
	if ((figureData.dim>=0) && (index>0) && (index<=figureData.count[0]))
		consoleCmdVertexSelected=index-1;
	else
		scriptThrowException("Wrong index");
}

void consoleCmdVertexNext() {
	if ((figureData.dim>=0) && (figureData.count[0]))
		consoleCmdVertexSelected=(consoleCmdVertexSelected+1)%figureData.count[0];
	else
		scriptThrowException("No vertex exist");
}

void consoleCmdVertexPrevious() {
	if ((figureData.dim>=0) && (figureData.count[0]))
		if (consoleCmdVertexSelected==-1)
			consoleCmdVertexSelected=figureData.count[0]-1;
		else
			consoleCmdVertexSelected=(consoleCmdVertexSelected+figureData.count[0]-1)%figureData.count[0];
	else
		scriptThrowException("No vertex exist");
}

void consoleCmdVertexDeselect() {
	consoleCmdVertexSelected=-1;
}

void consoleCmdVertexMove(int coordsCnt, float *coords) {
	if (coordsCnt>figureData.dim) {
		scriptThrowException("Too many arguments");
		return;
	}
	if (consoleCmdVertexSelected<0) {
		scriptThrowException("Nothing selected");
		return;
	}
	GLfloat shift[figureData.dim];
	GLfloat pos[figureData.dim];
	
	int i=0;
	for (; i<coordsCnt; i++)
		shift[i]=coords[i];
	for (; i<figureData.dim; i++)
		shift[i]=0;

	matrixProduct(shift, figureRotMatrix, pos, 1, figureData.dim, figureData.dim);
	matrixAdd(pos, figureData.vertices[consoleCmdVertexSelected], figureData.dim);
	if (!safeCheckPos(pos, figureData.dim)) {
		scriptThrowException("Wrong position");
		return;
	}
	figureVertexMove(consoleCmdVertexSelected, pos);
}

void consoleCmdVertexAdd(int coordsCnt, float *coords) {
	if (coordsCnt>figureData.dim) {
		scriptThrowException("Too many arguments");
		return;
	}
	if (figureData.dim<0) {
		scriptThrowException("There is no space yet, use new or open");
		return;
	}
	GLfloat pos[figureData.dim];
	GLfloat pos2[figureData.dim];

	int i=0;
	for (; i<coordsCnt; i++)
		pos[i]=coords[i];
	for (; i<figureData.dim; i++)
		pos[i]=0;

	matrixProduct(pos, figureRotMatrix, pos2, 1, figureData.dim, figureData.dim);
	if (!safeCheckPos(pos2, figureData.dim)) {
		scriptThrowException("Wrong position");
		return;
	}

	consoleCmdVertexSelected=figureVertexAdd(pos2);
}

void consoleCmdVertexRm() {
	if (consoleCmdVertexSelected<0) {
		scriptThrowException("Nothing selected");
		return;
	}
	figureVertexRm(consoleCmdVertexSelected);
	consoleCmdVertexSelected=-1;
}
