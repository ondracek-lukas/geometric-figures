// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmd.h"

#include <stdlib.h>

#include "figure.h"
#include "console.h"
#include "matrix.h"
#include "safe.h"

int consoleCmdVertexSelected=-1;

void consoleCmdVertexSelect(char *param) {
	int i=strtol(param, &param, 10);
	if ((*param=='\0') && (i>0) && (i<=figureData.count[0]))
		consoleCmdVertexSelected=i-1;
	else
		consolePrintErr("Wrong parameters.");
}

void consoleCmdVertexNext() {
	if ((figureData.dim>=0) && (figureData.count[0]))
		consoleCmdVertexSelected=(consoleCmdVertexSelected+1)%figureData.count[0];
	else
		consolePrintErr("No vertex exists");
}

void consoleCmdVertexPrevious() {
	if ((figureData.dim>=0) && (figureData.count[0]))
		if (consoleCmdVertexSelected==-1)
			consoleCmdVertexSelected=figureData.count[0]-1;
		else
			consoleCmdVertexSelected=(consoleCmdVertexSelected+figureData.count[0]-1)%figureData.count[0];
	else
		consolePrintErr("No vertex exists");
}

void consoleCmdVertexDeselect() {
	consoleCmdVertexSelected=-1;
}

static bool readCoordinates(char *str, GLfloat *coords) {
	int i;
	for (i=0; (i<figureData.dim) && (*str!='\0'); i++)
		coords[i]=strtof(str, &str);
	if (*str!='\0') {
		return false;
	}
	for (; i<figureData.dim; i++)
		coords[i]=0;
	return true;
}

void consoleCmdVertexMove(char *params) {
	if (consoleCmdVertexSelected<0) {
		consolePrintErr("Nothing selected");
		return;
	}
	GLfloat shift[figureData.dim];
	GLfloat pos[figureData.dim];
	
	if (!readCoordinates(params, shift)) {
		consolePrintErr("Wrong parameters");
		return;
	}

	matrixProduct(shift, figureRotMatrix, pos, 1, figureData.dim, figureData.dim);
	matrixAdd(pos, figureData.vertices[consoleCmdVertexSelected], figureData.dim);
	if (!safeCheckPos(pos, figureData.dim)) {
		consolePrintErr("Wrong position");
		return;
	}
	figureVertexMove(consoleCmdVertexSelected, pos);
}

void consoleCmdVertexAdd(char *params) {
	if (figureData.dim<0) {
		consolePrintErr("There is no space yet, use new or open");
		return;
	}
	GLfloat pos[figureData.dim];
	GLfloat pos2[figureData.dim];

	if (!readCoordinates(params, pos)) {
		consolePrintErr("Wrong parameters");
		return;
	}

	matrixProduct(pos, figureRotMatrix, pos2, 1, figureData.dim, figureData.dim);
	if (!safeCheckPos(pos2, figureData.dim)) {
		consolePrintErr("Wrong position");
		return;
	}

	consoleCmdVertexSelected=figureVertexAdd(pos2);
}

void consoleCmdVertexRm() {
	if (consoleCmdVertexSelected<0) {
		consolePrintErr("Nothing selected");
		return;
	}
	figureVertexRm(consoleCmdVertexSelected);
	consoleCmdVertexSelected=-1;
}
