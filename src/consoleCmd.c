// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmd.h"

#include <stdio.h>
#include <string.h>

#include "console.h"
#include "consoleCmds.h"
#include "util.h"
#include "figure.h"
#include "drawer.h"
#include "safe.h"
#include "hid.h"
#include "anim.h"
#include "script.h"

void consoleCmdSource(char *path) {
	path=utilExpandPath(path);
	scriptExecFile(path);
}

void consoleCmdOpen(char *path) {
	consoleCmdVertexDeselect();
	struct figureData *figure=figureRead(utilExpandPath(path));
	if (!figure) {
		scriptThrowException("File cannot be opened or has wrong format");
		return;
	}

	figureOpen(figure, false);
}

void consoleCmdWrite(char *path) {
	if (figureData.dim<0)
		scriptThrowException("Nothing opened");
	else if (!figureWrite(utilExpandPath(path), &figureData, true))
		scriptThrowException("File cannot be opened for writing");
}

void consoleCmdNew(int dim) {
	if ((dim<0) || (dim>safeMaxDim)) {
		scriptThrowException("Wrong parameters");
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

void consoleCmdRotate(int axis1, int axis2, float angle) {
	if ((axis1<1) || (axis2<1) || (axis1>figureData.dim) || (axis2>figureData.dim) || (axis1==axis2)) {
		scriptThrowException("Wrong parameters");
		return;
	}
	figureRotate(axis1-1, axis2-1, angle);
}

void consoleCmdMap(char *key, char *cmd_or_expr) {
	int code=hidCodeFromString(key);
	if (!code) {
		scriptThrowException("Wrong key shortcut");
		return;
	}
	char *expr=consoleCmdsToScriptExpr(cmd_or_expr);
	if (!expr)
		expr=cmd_or_expr;
	hidMap(code, expr);
}

void consoleCmdUnmap(char *key) {
	int code=hidCodeFromString(key);
	if (!code) {
		scriptThrowException("Wrong key shortcut");
		return;
	}
	hidMap(code, 0);
}

void consoleCmdRmap(char *key, int axis1, int axis2) {
	int code=hidCodeFromString(key);
	if (!code) {
		scriptThrowException("Wrong key shortcut");
		return;
	}

	if ((axis1<1) || (axis2<1) || (axis1==axis2)) {
		scriptThrowException("Wrong parameters");
		return;
	}
	animDestroyRot(hidMapRot(code, animCreateRot(axis1-1, axis2-1)));
}

void consoleCmdRunmap(char *key) {
	int code=hidCodeFromString(key);
	if (!code) {
		scriptThrowException("Wrong key shortcut");
		return;
	}
	hidMapRot(code, NULL);
}

void consoleCmdHelp(char *name) {
	if (!consolePrintBlock("help", name))
		scriptThrowException("Wrong name of help page");
}

void consoleCmdHistory() {
	consoleAppendMode();
	consolePrint("--- History of commands ---");
	consolePrintLinesList(consoleGetHistory());
	consoleClearAfterCmdDefaultMsg();
}

void consoleCmdQuit() {
	glutLeaveMainLoop();
}
