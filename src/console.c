// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "console.h"
#include "consolePrivDraw.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>

#include "figure.h"
#include "util.h"
#include "safe.h"
#include "drawer.h"
#include "convex.h"
#include "strings.h"
#include "anim.h"
#include "debug.h"
#include "matrix.h"
#include "keyboard.h"
#include "consoleCmd.h"
#include "consoleCmds.h"
#include "script.h"

enum consolePrintMode consolePrintMode=consolePrintOneLine;

int consoleInLength=0;
char *consoleIn=0;
char *consoleOut=0;
char *consoleStatus=0;

struct utilStrList *consolePrintMultilineList=0;
static bool multilineListDestroy;

static struct utilStrList *historyFirst=0;
static struct utilStrList *historyLast=0;
static struct utilStrList *historyActive=0;
static int historyCount=0;
static int historyMaxCount=20;

static void initCmds();
void consoleInit() {
	utilStrRealloc(&consoleIn, 0, 32);
	utilStrRealloc(&consoleOut, 0, 32);
	utilStrRealloc(&consoleStatus, 0, 32);
	*consoleIn='\0';
	*consoleStatus='\0';
	*consoleOut='\0';

	initCmds();
}

struct utilStrList *consoleBlock=0;
int consoleBlockWidth;
int consoleBlockHeight;
bool consolePrintBlock(char *section, char *name) {
	consoleClearBlock();
	consoleBlock=stringsGet(section, name, &consoleBlockWidth, &consoleBlockHeight);
	return consoleBlock;
}

void consoleClearBlock() {
	while (consoleBlock)
		consoleBlock=utilStrListRm(consoleBlock);
}


bool consolePrintStatus(char *string) {
	if (strcmp(consoleStatus, string)!=0) {
		utilStrRealloc(&consoleStatus, 0, strlen(string)+1);
		strcpy(consoleStatus, string);
		return true;
	} else {
		return false;
	}
}

void consolePrintMultilineAtOnce(struct utilStrList *lines, bool destroy) {
	consolePrintMultilineBegin();
	consolePrintMultilineList=lines;
	multilineListDestroy=destroy;
}

void consolePrintMultilineBegin() {
	consoleClear();
	consolePrintMode=consolePrintMultiline;
	multilineListDestroy=false;
}

void consolePrint(char *string) {
	switch(consolePrintMode) {
		case consolePrintMultiline:
			consolePrintMultilineList=utilStrListAddAfter(consolePrintMultilineList);
			utilStrRealloc(&consolePrintMultilineList->str, 0, strlen(string)+1);
			strcpy(consolePrintMultilineList->str, string);
			break;
		default:
			consoleClear();
			utilStrRealloc(&consoleOut, 0, strlen(string)+1);
			strcpy(consoleOut, string);
			break;
	}
}

void consolePrintErr(char *string) {
	consoleClear();
	consolePrintMode=consolePrintOneLineErr;
	utilStrRealloc(&consoleOut, 0, strlen(string)+1);
	strcpy(consoleOut, string);
}

void consoleClear() {
	historyActive=0;
	*consoleOut='\0';
	*consoleIn='\0';
	consoleInLength=0;
	consolePrintMode=consolePrintOneLine;
	if (multilineListDestroy)
		while (consolePrintMultilineList)
			consolePrintMultilineList=utilStrListRm(consolePrintMultilineList);
	else
		consolePrintMultilineList=0;
}

void consoleKeyPress(char c) {
	utilStrRealloc(&consoleIn, 0, ++consoleInLength+1);
	consoleIn[consoleInLength-1]=c;
	consoleIn[consoleInLength]='\0';
}

void consoleBackspace() {
	consoleIn[--consoleInLength]='\0';
}

void consoleEnter() {
	struct utilStrList *node;
	if (historyMaxCount) {
		if (historyCount<historyMaxCount) {
			node=safeMalloc(sizeof(struct utilStrList));
			node->str=0;
			historyCount++;
		} else {
			node=historyFirst;
			historyFirst=node->next;
			if (historyFirst)
				historyFirst->prev=0;
			else
				historyLast=0;
		}
		node->next=0;
		node->prev=historyLast;
		if (historyLast)
			historyLast->next=node;
		else
			historyFirst=node;
		historyLast=node;
		utilStrRealloc(&node->str, 0, consoleInLength+1);
		strcpy(node->str, consoleIn);
	}
	consoleInLength=0; // hide command before execution
	consoleExecuteCmd(consoleIn+1); // skip :
	if ((consolePrintMode==consolePrintOneLine) && (consoleOut[0]=='\0'))
		consoleClear();
}

void consoleUp() {
	char *s;
	if (historyActive==0) {
		historyActive=historyLast;
	}
	else if (historyActive->prev)
		historyActive=historyActive->prev;
	if (historyActive!=0) {
		strcpy(consoleIn, historyActive->str);
		consoleInLength=0;
		for (s=consoleIn; *s; s++)
			consoleInLength++;
	}
}

void consoleDown() {
	char *s;
	if (historyActive!=0) {
		historyActive=historyActive->next;
		if (historyActive==0) {
			consoleIn[1]='\0';
			consoleInLength=1;
		} else {
			strcpy(consoleIn, historyActive->str);
			consoleInLength=0;
			for (s=consoleIn; *s; s++)
				consoleInLength++;
		}
	}
}

#define addNew(prefix,params,expr) lastParams=params; lastExpr=expr; consoleCmdsAdd(prefix,lastParams,lastExpr)
#define addAlias(prefix) consoleCmdsAdd(prefix,lastParams,lastExpr)
void initCmds() {
	int lastParams;
	char *lastExpr;
	addNew  ("help",             0, "gf.help(\"\")"          );
	addNew  ("help ",            1, "gf.help(\"%\")"         );
	addNew  ("echo ",            1, "gf.echo(\"%\")"         );
	addNew  ("history",          0, "gf.history()"           );
	addNew  ("map ",             1, "gf.map(\"%\")"          );
	addNew  ("new ",             1, "gf.new(\"%\")"          );
	addAlias("n ");
	addNew  ("close",            0, "gf.close()"             );
	addNew  ("open ",            1, "gf.open(\"%\")"         );
	addAlias("o ");
	addNew  ("quit",             0, "gf.quit()"              );
	addAlias("q");
	addAlias("exit");
	addNew  ("rotate ",          1, "gf.rotate(\"%\")"       );
	addAlias("rot ");
	addNew  ("reset rotation",   0, "gf.resetRotation()"     );
	addAlias("reset rot");
	addNew  ("reset colors",     0, "gf.resetColors()"       );
	addNew  ("reset boundary",   0, "gf.resetBoundary()"     );
	addNew  ("rmap ",            1, "gf.rmap(\"%\")"         );
	addNew  ("set",              0, "gf.set(\"\")"           );
	addNew  ("set ",             1, "gf.set(\"%\")"          );
	addNew  ("source ",          1, "gf.source(\"%\")"       );
	addAlias("so ");
	addNew  ("vertex add ",      1, "gf.vertexAdd(\"%\")"    );
	addAlias("vert add");
	addNew  ("vertex deselect",  0, "gf.vertexDeselect()"    );
	addAlias("vertex desel");
	addAlias("vert deselect");
	addAlias("vert desel");
	addNew  ("vertex move ",     1, "gf.vertexMove(\"%\")"   );
	addAlias("vert move");
	addNew  ("vertex next",      0, "gf.vertexNext()"        );
	addAlias("vert next");
	addNew  ("vertex previous",  0, "gf.vertexPrevious()"    );
	addAlias("vertex prev");
	addAlias("vert previous");
	addAlias("vert prev");
	addNew  ("vertex remove",    0, "gf.vertexRemove()"      );
	addAlias("vertex rm");
	addAlias("vert remove");
	addAlias("vert rm");
	addNew  ("vertex select ",   1, "gf.vertexSelect(\"%\")" );
	addAlias("vertex sel");
	addAlias("vert select");
	addAlias("vert sel");
	addNew  ("write ",           1, "gf.write(\"%\")"        );
	addAlias("w ");
}
#undef addNew
#undef addAlias

static void executeBlock(char *cmds);
void consoleExecuteCmd(char *cmd) {
	if (*cmd=='{') {
		executeBlock(cmd+1);
		return;
	} else {
		char *expr=consoleCmdsToScriptExpr(cmd);
		if (expr) {
			char *ret=scriptEvalExpr(expr);
			if (ret && *ret)
				consolePrint(ret);
			return;
		} else {
			scriptThrowException("Wrong command or missing parameter");
		}
	}

	char *err=scriptCatchException();
	if (err)
		consolePrintErr(err);
}

static void executeBlock(char *cmds) {
	char *cmd=cmds;
	int braces=1;
	while (braces) {
		switch(*cmds) {
			case ' ':
			case '\n':
				if (cmd==cmds)
					cmd++;
				break;
			case '{':
				braces++;
				break;
			case '}':
				if (braces==1) {
					*cmds='\0';
					consoleExecuteCmd(cmd);
					*cmds='}';
				}
				braces--;
				break;
			case ';':
				if (braces==1) {
					*cmds='\0';
					consoleExecuteCmd(cmd);
					*cmds=';';
					cmd=cmds+1;
				}
				break;
			case '\0':
				consoleExecuteCmd(cmd);
				braces=0;
				break;
		}
		cmds++;
	}
}

int consoleGetHistoryMaxCount() {
	return historyMaxCount;
}

void consoleSetHistoryMaxCount(int maxCount) {
	historyMaxCount=maxCount;
	for (; historyCount>historyMaxCount; historyCount--)
		if (historyCount>1) {
			historyFirst=historyFirst->next;
			utilStrRealloc(&historyFirst->prev->str, 0, 0);
			free(historyFirst->prev);
			historyFirst->prev=0;
		} else {
			free(historyFirst);
			historyFirst=0;
			historyLast=0;
		}
}

struct utilStrList *consoleGetHistory() {
	return historyLast;
}

bool consoleIsOpen() {
	return consoleInLength;
}
