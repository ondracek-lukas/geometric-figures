// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmd.h"

#include <stdio.h>
#include <string.h>

#include "console.h"
#include "util.h"
#include "figure.h"
#include "drawer.h"
#include "safe.h"
#include "keyboard.h"
#include "anim.h"
#include "script.h"

void consoleCmdSource(char *path) {
	path=utilExpandPath(path);
	scriptExecFile(path);
}

void consoleCmdOpen(char *path) {
	consoleCmdVertexDeselect();
	if (!figureOpen(utilExpandPath(path)))
		if (figureData.dim>=0)
			consolePrintErr("File cannot be opened");
		else
			consolePrintErr("File cannot be opened or has wrong format");
	drawerSetDim(figureData.dim);
}

void consoleCmdWrite(char *path) {
	if (figureData.dim<0)
		consolePrintErr("Nothing opened");
	else if (!figureSave(utilExpandPath(path)))
		consolePrintErr("File cannot be opened for writing");
}

void consoleCmdNew(int dim) {
	if ((dim<0) || (dim>safeMaxDim)) {
		consolePrintErr("Wrong parameters");
		return;
	}
	consoleCmdVertexDeselect();
	figureNew(dim);
	drawerSetDim(dim);
}

void consoleCmdClose() {
	consoleCmdVertexDeselect();
	figureNew(-1);
	drawerSetDim(-1);
}

void consoleCmdRotate(float axis1, float axis2, float angle) {
	if ((axis1<1) || (axis2<1) || (axis1>figureData.dim) || (axis2>figureData.dim) || (axis1==axis2)) {
		consolePrintErr("Wrong parameters");
		return;
	}
	figureRotate(axis1-1, axis2-1, angle);
}

void consoleCmdMap(char *key, char *cmd) {
	int code=keyboardCodeFromString(key);
	if (!code) {
		consolePrintErr("Wrong key shortcut");
		return;
	}
	keyboardMap(code, cmd);
}

void consoleCmdUnmap(char *key) {
	int code=keyboardCodeFromString(key);
	if (!code) {
		consolePrintErr("Wrong key shortcut");
		return;
	}
	keyboardMap(code, 0);
}

void consoleCmdRmap(char *key, int axis1, int axis2) {
	int code=keyboardCodeFromString(key);
	if (!code) {
		consolePrintErr("Wrong key shortcut");
		return;
	}

	if ((axis1<1) || (axis2<1) || (axis1==axis2)) {
		consolePrintErr("Wrong parameters");
		return;
	}
	animDestroyRot(keyboardMapRot(code, animCreateRot(axis1-1, axis2-1)));
}

void consoleCmdRunmap(char *key) {
	int code=keyboardCodeFromString(key);
	if (!code) {
		consolePrintErr("Wrong key shortcut");
		return;
	}
	keyboardMapRot(code, NULL);
}

void consoleCmdHelp(char *name) {
	if (!consolePrintBlock("help", name))
		consolePrintErr("Wrong name of help page");
}

void consoleCmdHistory() {
	consolePrintMultilineAtOnce(consoleGetHistory(), false);
}

void consoleCmdQuit() {
	glutLeaveMainLoop();
}
