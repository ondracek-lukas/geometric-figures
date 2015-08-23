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

static struct utilStrList *historyFirst;
static struct utilStrList *historyLast;
static struct utilStrList *historyActive;
static int historyCount;
static int historyMaxCount;
static int cmdEnd;
static int cmdBegin;
static int cursorPos;
static int cmdExecutionLevel;

static void initCmds();
void consoleInit() {
	utilStrRealloc(&consoleStatus, 0, 32);
	*consoleStatus='\0';

	initCmds();
}

int consoleStrWidth(char *str) {
	int width=0;
	int lineWidth=0;
	for (; *str; str++) {
		switch (*str) {
			case '\n':
				if (width<lineWidth)
					width=lineWidth;
				lineWidth=0;
				break;
			case consoleSpecialBack:
				lineWidth--;
				break;
			case consoleSpecialColorNormal:
			case consoleSpecialColorRed:
			case consoleSpecialColorGreen:
			case consoleSpecialColorGray:
				break;
			default:
				lineWidth++;
		}
	}
	if (width<lineWidth)
		width=lineWidth;
	return width;
}
int consoleStrHeight(char *str) {
	int height=1;
	for (; *str; str++)
		if (*str=='\n')
			height++;
	return height;
}


// -- history --

static struct utilStrList *historyFirst=0;
static struct utilStrList *historyLast=0;
static struct utilStrList *historyActive=0;
static int historyCount=0;
static int historyMaxCount=20;

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
	return historyFirst;
}



// -- block printing --

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
		utilStrListRm(&consoleBlock);
}


// -- status line --

char *consoleStatus=0;

bool consolePrintStatus(char *string) {
	if (strcmp(consoleStatus, string)!=0) {
		utilStrRealloc(&consoleStatus, 0, strlen(string)+1);
		strcpy(consoleStatus, string);
		return true;
	} else {
		return false;
	}
}


// -- command line printing --

struct utilStrList *consoleLines=0;
static enum mode {
	modeNormal,
	modeRewriteLast,
	modeAppend
} mode = modeNormal;

void consolePrint(char *str) {
	consolePrintLinesList(utilStrListOfLines(str));
}

void consolePrintLinesList(struct utilStrList *lines) {
	if (cmdExecutionLevel)
		mode=modeAppend;
	if (mode!=modeAppend)
		consoleClear();
	if (lines) {
		while (lines->prev)
			lines=lines->prev;
		utilStrListCopyAfter(&consoleLines, lines);

		struct utilStrList *lines2=consoleLines;
		while (lines=lines->next)
			lines2=lines2->prev;

		utilStrRealloc(&lines2->str, 0, strlen(lines2->str)+2);
		utilStrInsertChar(lines2->str, consoleSpecialColorNormal);
	}
}

void consolePrintErr(char *str) {
	static char *str2=0;
	utilStrRealloc(&str2, 0, strlen(str)+2);
	*str2=consoleSpecialColorRed;
	strcpy(str2+1, str);
	consolePrint(str2);
}

void consoleClear() {
	historyActive=0;
	cursorPos=0;
	mode=modeNormal;
	while (consoleLines)
		utilStrListRm(&consoleLines);
}

void consoleAppendMode() {
	mode=modeAppend;
}
void consoleClearBeforePrinting() {
	mode=modeNormal;
}
void consoleClearAfterCmdDefaultMsg() {
	consoleClearAfterCmd("Press any key or type command to continue");
}
void consoleClearAfterCmd(char *msg) {
	mode=modeAppend;
	consolePrint(msg);
	mode=modeRewriteLast;
}


// -- command line and key events handling --

static int cmdEnd=0;
static int cmdBegin=2;  // 0th is consoleSpecialColorNormal, 1st is :
static int completionEnd=0;
struct utilStrList *completions=0;

static char cursor[]={
	consoleSpecialColorGray,
	'\x14',
	consoleSpecialColorNormal,
	consoleSpecialBack,
	'\0'};
static char cursorLen=sizeof(cursor)-1;

static void cmdLineRealloc(int newChars) {
	utilStrRealloc(&consoleLines->str, 0, (cmdEnd>completionEnd?cmdEnd:completionEnd)+newChars+1);
}

static void setCursorPos(int pos) {
	if (cursorPos) {
		utilStrRmChars(consoleLines->str+cursorPos, cursorLen);
		if (cursorPos<cmdEnd)
			cmdEnd-=cursorLen;
		if (completionEnd) {
			completionEnd-=cursorLen;
			consoleLines->str[cmdEnd]=consoleSpecialColorGray;
		}
	} else if(pos) {
		cmdLineRealloc(cursorLen);
	}
	cursorPos=pos;
	if (cursorPos) {
		utilStrInsertChars(consoleLines->str+cursorPos, cursor);
		if (cursorPos<=cmdEnd)
			cmdEnd+=cursorLen;
		else
			consoleLines->str[cursorPos+2]=consoleSpecialColorGray;
		if (completionEnd) {
			completionEnd+=cursorLen;
			if (cursorPos>cmdEnd)
				consoleLines->str[cmdEnd]=consoleSpecialColorBlue;
		}
	}
}

static void openConsole(char *str) {
	if ((mode==modeRewriteLast) && consoleLines) {
		mode=modeAppend;
		utilStrListRm(&consoleLines);
		consolePrint(str);
		mode=modeRewriteLast;
	} else {
		consoleClear();
		consolePrint(str);
	}
	cmdEnd=strlen(consoleLines->str);
	setCursorPos(cmdEnd);
}

bool consoleIsOpen() {
	return cursorPos;
}

void showActiveCompletion() {
	consoleLines->str[cmdEnd]='\0';
	completionEnd=0;
	if (completions) {
		completionEnd=strlen(completions->str);
		cmdLineRealloc(completionEnd+2);
		consoleLines->str[cmdEnd]=consoleSpecialColorGray;
		strcpy(consoleLines->str+cmdEnd+1, completions->str);
		completionEnd+=1+cmdEnd;
	}
}

void updateCompletions() {
	consoleLines->str[cmdEnd]='\0';
	while (completions)
		utilStrListRm(&completions);
	completions=consoleCmdsComplete(consoleLines->str+cmdBegin);
	showActiveCompletion();
}

void applyCompletion() {
	if (cursorPos>cmdEnd) {
		int pos=cursorPos;
		setCursorPos(0);
		consoleLines->str[pos]='\0';
		utilStrRmChars(consoleLines->str+cmdEnd, 1);
		cmdEnd=pos-1;
		setCursorPos(cmdEnd);
	} else {
		consoleLines->str[cmdEnd]='\0';
	}
	completionEnd=0;
}


void consoleKeyPress(char c) {
	if (!cursorPos)
		openConsole("");
	applyCompletion();

	int pos=cursorPos;
	setCursorPos(0);
	cmdLineRealloc(1);
	utilStrInsertChar(consoleLines->str+pos, c);
	cmdEnd++;

	updateCompletions();

	setCursorPos(pos+1);
}

void consoleBackspace() {
	if (cursorPos>cmdBegin) {
		if (cursorPos<=cmdEnd) {
			int pos=cursorPos;
			setCursorPos(0);
			utilStrRmChars(consoleLines->str + --pos, 1);
			cmdEnd--;
			updateCompletions();
			setCursorPos(pos);
		} else {
			setCursorPos(cmdEnd);
		}
	} else if (cursorPos+cursorLen==cmdEnd)
		consoleClear();
}

void consoleEnter() {
	char *cmd=consoleLines->str;
	applyCompletion();
	cmd[cmdEnd]='\0';
	setCursorPos(0);
	
	if (historyMaxCount) {
		if (historyCount<historyMaxCount)
			historyCount++;
		else
			utilStrListRm(&historyFirst);
		utilStrListAddAfter(&historyLast);
		if (!historyFirst)
			historyFirst=historyLast;
		utilStrRealloc(&historyLast->str, 0, cmdEnd+1);
		strncpy(historyLast->str, cmd+1, cmdEnd-1);
		historyLast->str[cmdEnd-1]='\0';
	}
	consoleLines->str=0;
	consoleClear();
	consoleExecuteCmd(cmd+cmdBegin); // skip : and consoleSpecialColorNormal
	utilStrRealloc(&cmd, 0, 0);
}

void consoleUp() {
	if (!historyActive)
		historyActive=historyLast;
	else if (historyActive->prev)
		historyActive=historyActive->prev;
	if (historyActive) {
		struct utilStrList *active=historyActive;
		openConsole(active->str);
		historyActive=active;
	}
}

void consoleDown() {
	if (historyActive) {
		historyActive=historyActive->next;
		if (historyActive) {
			struct utilStrList *active=historyActive;
			openConsole(historyActive->str);
			historyActive=active;
		} else {
			openConsole(":");
		}
	}
}

void consoleLeft() {
	if (cursorPos>cmdBegin) {
		if (cursorPos==cmdEnd+1)
			setCursorPos(cursorPos-2);
		else
			setCursorPos(cursorPos-1);
	}
}

void consoleRight() {
	if ((cursorPos+cursorLen<cmdEnd) || (cursorPos+cursorLen<completionEnd)) {
		if (cursorPos+cursorLen==cmdEnd)
			setCursorPos(cursorPos+2);
		else
			setCursorPos(cursorPos+1);
	}
}

void consoleTab() {
	if (completions) {
		int pos=cursorPos;
		setCursorPos(0);
		if (pos==completionEnd) {
			if (completions->next)
				completions=completions->next;
			else while (completions->prev)
				completions=completions->prev;
			showActiveCompletion();
		}
		if (*completions->str=='\0')
			setCursorPos(cmdEnd);
		else
			setCursorPos(completionEnd);
	}
}


// -- commands --

static int cmdExecutionLevel=0; // number of consoleEvalExpr in stacktrace
bool consoleAllowPythonExpr=false;
bool consolePythonExprToStdout=false;

#define addNew(prefix, expr, params, paramsFlags) \
	lastParams=params; \
	lastExpr=expr; \
	lastParamsFlags=paramsFlags; \
	consoleCmdsAdd(prefix, lastExpr, lastParams, lastParamsFlags, false)
#define addAlias(prefix) consoleCmdsAdd(prefix, lastExpr, lastParams, lastParamsFlags, true)
void initCmds() {
	int lastParams;
	char *lastParamsFlags;
	char *lastExpr;

	addNew  ("help",            "gf.help(\"\")",           0, ""     );
	const char * const *helpPages=stringsGetContent("help");
	while (*helpPages) {
		char cmd[50], expr[50];
		sprintf(cmd, "help %s", *helpPages);
		sprintf(expr, "gf.help(\"%s\")", *helpPages);
		addNew(cmd,               expr,                      0, ""     );
		helpPages++;
	}

	addNew  ("echo ",           "gf.echo(%)",              1, "s"    );
	addNew  ("history",         "gf.history()",            0, ""     );
	addNew  ("map ",            "gf.map(%)",              -1, "s"    );
	addNew  ("new ",            "gf.new(%)",               1, ""     );
	addAlias("n ");
	addNew  ("close",           "gf.close()",              0, ""     );
	addNew  ("open ",           "gf.open(%)",              1, "p"    );
	addAlias("o ");
	addNew  ("quit",            "gf.quit()",               0, ""     );
	addAlias("q");
	addAlias("exit");
	addNew  ("rotate ",         "gf.rotate(%)",           -1, ""     );
	addAlias("rot ");
	addNew  ("reset rotation",  "gf.resetRotation()",      0, ""     );
	addAlias("reset rot");
	addNew  ("reset colors",    "gf.resetColors()",        0, ""     );
	addNew  ("reset boundary",  "gf.resetBoundary()",      0, ""     );
	addNew  ("rmap ",           "gf.rmap(%)",             -1, "s-"   );

	consoleCmdSetUpdateCmds();

	addNew  ("source ",         "gf.source(%)",            1, "p"    );
	addAlias("so ");
	addNew  ("vertex add",      "gf.vertexAdd()",          0, ""     );
	addAlias("vert add");
	addNew  ("vertex add ",     "gf.vertexAdd(%)",        -1, ""     );
	addAlias("vert add ");
	addNew  ("vertex deselect", "gf.vertexDeselect()",     0, ""     );
	addAlias("vertex desel");
	addAlias("vert deselect");
	addAlias("vert desel");
	addNew  ("vertex move ",    "gf.vertexMove(%)",       -1, ""    );
	addAlias("vert move ");
	addNew  ("vertex next",     "gf.vertexNext()",         0, ""     );
	addAlias("vert next");
	addNew  ("vertex previous", "gf.vertexPrevious()",     0, ""     );
	addAlias("vertex prev");
	addAlias("vert previous");
	addAlias("vert prev");
	addNew  ("vertex remove",   "gf.vertexRemove()",       0, ""     );
	addAlias("vertex rm");
	addAlias("vert remove");
	addAlias("vert rm");
	addNew  ("vertex select ",  "gf.vertexSelect(%)",      1, ""     );
	addAlias("vertex sel ");
	addAlias("vert select ");
	addAlias("vert sel ");
	addNew  ("write ",          "gf.write(%)",             1, "p"    );
	addAlias("w ");

	consoleCmdsAddColor("black",       "#000000",  false);
	consoleCmdsAddColor("white",       "#ffffff",  false);
	consoleCmdsAddColor("red",         "#ff0000",  false);
	consoleCmdsAddColor("green",       "#00ff00",  false);
	consoleCmdsAddColor("blue",        "#0000ff",  false);
	consoleCmdsAddColor("yellow",      "#ffff00",  false);
	consoleCmdsAddColor("cyan",        "#00ffff",  false);
	consoleCmdsAddColor("purple",      "#ff00ff",  false);
	consoleCmdsAddColor("gray",        "#808080",  false);
	consoleCmdsAddColor("transparent", "#00000000", true);
}
#undef addNew
#undef addAlias

void consoleExecuteCmd(char *cmd) {
	static char *tmp=0;
	char *tmp2, *expr;
	utilStrRealloc(&tmp, 0, strlen(cmd)+1);
	while (*cmd==' ')
		cmd++;
	tmp2=stpcpy(tmp, cmd);
	while (*--tmp2==' ');
	*++tmp2='\0';
	cmd=tmp;

	expr=consoleCmdsToScriptExpr(cmd);
	if (!expr && consoleAllowPythonExpr)
		expr=cmd;
	if (expr) {
		consoleEvalExpr(expr);
	} else {
		consolePrintErr("Wrong command or missing parameter");
	}
}


static void execFile(int str_path);
static char *execFilePath=0;
void consoleExecFile(char *path) {
	utilStrRealloc(&execFilePath, 0, strlen(path)+1);
	strcpy(execFilePath, path);
	glutTimerFunc(0, execFile, 0);
}

void execFile(int ignored) {
	consoleCmdSource(execFilePath);
	char *err=scriptCatchException();
	if (err)
		consolePrintErr(err);
}

static char *evalExprStr=0;
static void evalExpr(int str_expr);
void consoleEvalExpr(char *expr) {
	utilStrRealloc(&evalExprStr, 0, strlen(expr)+1);
	strcpy(evalExprStr, expr);
	glutTimerFunc(0, evalExpr, 0);
}

void evalExpr(int ignored) {
	cmdExecutionLevel++;

	char *ret=scriptEvalExpr(evalExprStr);
	if (ret && *ret)
		consolePrint(ret);

	char *err=scriptCatchException();
	if (err)
		consolePrintErr(err);

	if (consolePythonExprToStdout) {
		if (err)
			printf("%s # Error: %s\n", evalExprStr, err);
		else if (ret && *ret)
			printf("%s # Ret: %s\n", evalExprStr, ret);
		else
			printf("%s\n", evalExprStr);
	}

	cmdExecutionLevel--;
}
