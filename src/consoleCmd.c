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

void consoleCmdSource(char *path) {
	path=utilExpandPath(path);
	FILE *file=fopen(path, "r");
	char *line=0, *lineBegin, *lineEnd=0, *cmd=0, *cmdEnd=0;
	int braces=0;
	if (file==0) {
		consolePrintErr("File cannot be opened");
		return;
	}
	utilStrRealloc(&line, 0, 64);
	while (lineEnd=fgets(line, 64, file)) {
		while ((lineEnd=strchr(lineEnd, '\0'))[-1]!='\n') {
			utilStrRealloc(&line, &lineEnd, 64);
			if (!fgets(lineEnd, 64, file))
				break;
		}
		lineBegin=line;
		while ((*lineBegin==' ') || (*lineBegin=='\t') || (*lineBegin=='\r'))
			lineBegin++;
		lineEnd=lineBegin;
		while ((*lineEnd!='\0') && (*lineEnd!='\n') && (*lineEnd!='\r') && (*lineEnd!='#'))
			lineEnd++;
		*lineEnd='\0';

		if (cmd!=cmdEnd)
			*(cmdEnd++)=';';
		utilStrRealloc(&cmd, &cmdEnd, lineEnd-lineBegin+1);
		strcpy(cmdEnd, lineBegin);
		while (*cmdEnd!='\0')
			switch(*(cmdEnd++)) {
				case '{': braces++; break;
				case '}': (braces>0) && braces--; break;
			}
		if (braces<=0) {
			consoleExecuteCmd(cmd);
			cmdEnd=cmd;
			braces=0;
		}
	}
	if (cmd!=cmdEnd)
		consoleExecuteCmd(cmd);
	utilStrRealloc(&line, 0, 0);
	utilStrRealloc(&cmd, 0, 0);
	fclose(file);
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

void consoleCmdNew(char *str) {
	int dim=-1;
	if (*str!='\0') {
		dim=strtol(str, &str, 10);
		if ((*str!='\0') || (dim<0) || (dim>safeMaxDim)) {
			consolePrintErr("Wrong parameters");
			return;
		}
	}
	consoleCmdVertexDeselect();
	figureNew(dim);
	drawerSetDim(dim);
}

void consoleCmdRotate(char *str) {
	static int axis1=1, axis2=2;
	static GLfloat angle=0;
	if (*str!='\0') {
		axis1=strtol(str, &str, 10);
		axis2=strtol(str, &str, 10);
		angle=strtof(str, &str);
	}
	if ((*str!='\0') || (axis1<1) || (axis2<1) || (axis1>figureData.dim) || (axis2>figureData.dim) || (axis1==axis2)) {
		consolePrintErr("Wrong parameters");
		return;
	}
	figureRotate(axis1-1, axis2-1, angle);
}

static int chopKeyCode(char **str) {
	int code;
	char *space;
	space=strchr(*str, ' ');
	if (space!=0)
		*space='\0';
	code=keyboardCodeFromString(*str);
	if (space!=0)
		*space=' ';

	*str=space+1;
	return code;
}


void consoleCmdMap(char *str) {
	int code;

	code=chopKeyCode(&str);

	if (!code) {
		consolePrintErr("Wrong shortcut");
		return;
	}

	if (*str)
		keyboardMap(code, str);
	else
		keyboardMap(code, 0);
}

void consoleCmdRmap(char *str) {
	int code, axis1, axis2;

	code=chopKeyCode(&str);

	if (!code) {
		consolePrintErr("Wrong shortcut");
		return;
	}

	if (*str) {
		axis1=strtol(str, &str, 10);
		axis2=strtol(str, &str, 10);
		if ((*str!='\0') || (axis1<1) || (axis2<1) || (axis1==axis2)) {
			consolePrintErr("Wrong parameters");
			return;
		}
		animDestroyRot(keyboardMapRot(code, animCreateRot(axis1-1, axis2-1)));
	} else {
		keyboardMapRot(code, NULL);
	}
}

void consoleCmdHelp(char *name) {
	if (!consolePrintBlock("help", name))
		consolePrintErr("Wrong name of help page");
}

void consoleCmdHistory() {
	consolePrintMultilineAtOnce(consoleGetHistory(), false);
}
