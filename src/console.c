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

void consoleInit() {
	utilStrRealloc(&consoleIn, 0, 32);
	utilStrRealloc(&consoleOut, 0, 32);
	utilStrRealloc(&consoleStatus, 0, 32);
	*consoleIn='\0';
	*consoleStatus='\0';
	*consoleOut='\0';
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

static void executeBlock(char *cmds);

#define cmdl(p, s) (strcmp(part[p], s)==0)
	// compare part p of command with s
#define cmd(p, s) ((strncmp(s, part[p], sizeof(s)-1)==0)?(part[p+1]=part[p]+sizeof(s)-1, 1):0)
	// compare the beginning of part p of command with s, part[p+1]=rest of part[p]
#define cmdw(p, s) (cmd(p, s" ") || (cmd(p, s"\0") && (part[p+1]--)))
	// like cmd, s have to be terminated word in command
void consoleExecuteCmd(char *cmd) {
	DEBUG_CMDS(printf("%s\n", cmd);)
	char *part[4];
	for (part[0]=strchr(cmd, '\0'); (part[0]>cmd) && (part[0][-1]==' '); part[0]--);
	part[0][0]='\0';
	part[0]=cmd;

	if (*cmd=='\0') {}
	else if (*cmd=='{')
		executeBlock(cmd+1);
	else if cmdw(0, "set")
		consoleCmdSet(part[1]);
	else if (cmdw(0, "rot") || cmdw(0, "rotate"))
		consoleCmdRotate(part[1]);
	else if cmdw(0, "map")
		consoleCmdMap(part[1]);
	else if cmdw(0, "rmap")
		consoleCmdRmap(part[1]);
	else if (cmdw(0, "o") || cmdw(0, "open"))
		consoleCmdOpen(part[1]);
	else if (cmdw(0, "n") || cmdw(0, "new"))
		consoleCmdNew(part[1]);
	else if (cmdw(0, "w") || cmdw(0, "write"))
		consoleCmdWrite(part[1]);
	else if (cmdw(0, "so") || cmdw(0, "source"))
		consoleCmdSource(part[1]);
	else if cmdw(0, "echo")
		consolePrint(part[1]);
	else if (cmdw(0, "vert") || cmdw(0, "vertex")) {
		if (figureData.dim<0)
			consolePrintErr("Nothing opened");
		else if (*part[1]=='\0')
			consolePrintErr("Argument needed");
		else if (cmdw(1, "sel") || cmdw(1, "select"))
			consoleCmdVertexSelect(part[2]);
		else if (cmdl(1, "desel") || cmdl(1, "deselect"))
			consoleCmdVertexDeselect();
		else if cmdl(1, "next")
			consoleCmdVertexNext();
		else if (cmdl(1, "prev") || cmdl(1, "previous"))
			consoleCmdVertexPrevious();
		else if cmdw(1, "move")
			consoleCmdVertexMove(part[2]);
		else if cmdw(1, "add")
			consoleCmdVertexAdd(part[2]);
		else if (cmdl(1, "rm") || cmdl(1, "remove"))
			consoleCmdVertexRm();
		else
			consolePrintErr("Wrong subcommand");
	} else if cmdw(0, "reset") {
		if (*part[1]=='\0')
			consolePrintErr("Argument needed: rot[ation], boundary, colors");
		else if cmdl(1, "colors")
			drawerResetColors();
		else if (figureData.dim<0)
			consolePrintErr("Norhing opened");
		else if (cmdl(1, "rot") || cmdl(1, "rotation"))
			figureResetRotation();
		else if cmdl(1, "boundary")
			figureResetBoundary();
		else
			consolePrintErr("Wrong command");
	} else if cmdw(0, "help")
		consoleCmdHelp(part[1]);
	else if cmdl(0, "history")
		consoleCmdHistory();
	else if (cmdl(0, "q") || cmdl(0, "quit") || cmdl(0, "exit")) {
		glutLeaveMainLoop();
	} else {
		consolePrintErr("Wrong command");
	}
}

#undef cmdw
#undef cmd
#undef cmdl

static void executeBlock(char *cmds) {
	char *cmd=cmds;
	int braces=1;
	while (braces) {
		switch(*cmds) {
			case ' ':
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
