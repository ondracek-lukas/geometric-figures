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
static int cursorPos=0;
static const int cursorLen=4;

static void cmdLineRealloc(int newChars) {
	utilStrRealloc(&consoleLines->str, 0, cmdEnd+newChars);
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
	cursorPos=cmdEnd;
	cmdLineRealloc(2);
	consoleLines->str[cmdEnd++]=consoleSpecialColorGray;
	consoleLines->str[cmdEnd++]='\2'; // !! ? x14 = _
	consoleLines->str[cmdEnd++]=consoleSpecialColorNormal;
	consoleLines->str[cmdEnd++]=consoleSpecialBack;
	consoleLines->str[cmdEnd]='\0';
}

bool consoleIsOpen() {
	return cursorPos;
}

void consoleKeyPress(char c) {
	if (!cursorPos)
		openConsole("");
	cmdLineRealloc(1);
	utilStrInsertChar(consoleLines->str+cursorPos++, c);
	cmdEnd++;
}

void consoleBackspace() {
	if (cursorPos>cmdBegin) {
		utilStrRmChars(consoleLines->str + --cursorPos,1);
		cmdEnd--;
	} else if (cursorPos+cursorLen==cmdEnd)
		consoleClear();
}

void consoleEnter() {
	char *cmd=consoleLines->str;
	cmd[cmdEnd]='\0';
	utilStrRmChars(cmd+cursorPos,cursorLen);
	cmdEnd-=cursorLen;
	
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
		char c=consoleLines->str[cursorPos-1];
		cursorPos--;
		for (int i=0; i<cursorLen; i++)
			consoleLines->str[cursorPos+i]=consoleLines->str[cursorPos+i+1];
		consoleLines->str[cursorPos+cursorLen]=c;
	}
}

void consoleRight() {
	if (cursorPos+cursorLen<cmdEnd) {
		char c=consoleLines->str[cursorPos+cursorLen];
		cursorPos++;
		for (int i=cursorLen-1; i>=0; i--)
			consoleLines->str[cursorPos+i]=consoleLines->str[cursorPos+i-1];
		consoleLines->str[cursorPos-1]=c;
	}
}


// -- commands --

static int cmdExecutionLevel=0; // number of consoleEvalExpr in stacktrace
bool consoleAllowPythonExpr=false;
bool consolePythonExprToStdout=false;

#define addNew(prefix, params, paramsFlags, expr) \
	lastParams=params; \
	lastExpr=expr; \
	lastParamsFlags=paramsFlags; \
	consoleCmdsAdd(prefix, lastParams, lastParamsFlags, lastExpr)
#define addAlias(prefix) consoleCmdsAdd(prefix, lastParams, lastParamsFlags, lastExpr)
void initCmds() {
	int lastParams;
	char *lastParamsFlags;
	char *lastExpr;

	addNew  ("help",             0, "",     "gf.help(\"\")"          );
	const char * const *helpPages=stringsGetContent("help");
	int i=0;
	while (*helpPages) {
		char cmd[50], expr[50];
		sprintf(cmd, "help %s", *helpPages);
		sprintf(expr, "gf.help(\"%s\")", *helpPages);
		addNew(cmd,                0, "",     expr                     );
		helpPages++;
	}

	addNew  ("echo ",            1, "s",    "gf.echo(%)"             );
	addNew  ("history",          0, "",     "gf.history()"           );
	addNew  ("map ",            -1, "s",    "gf.map(%)"              );
	addNew  ("new ",             1, "",     "gf.new(%)"              );
	addAlias("n ");
	addNew  ("close",            0, "",     "gf.close()"             );
	addNew  ("open ",            1, "s",    "gf.open(%)"             );
	addAlias("o ");
	addNew  ("quit",             0, "",     "gf.quit()"              );
	addAlias("q");
	addAlias("exit");
	addNew  ("rotate ",         -1, "",     "gf.rotate(%)"           );
	addAlias("rot ");
	addNew  ("reset rotation",   0, "",     "gf.resetRotation()"     );
	addAlias("reset rot");
	addNew  ("reset colors",     0, "",     "gf.resetColors()"       );
	addNew  ("reset boundary",   0, "",     "gf.resetBoundary()"     );
	addNew  ("rmap ",           -1, "s-",   "gf.rmap(%)"             );

	consoleCmdSetUpdateCmds();

	addNew  ("source ",          1, "s",    "gf.source(%)"           );
	addAlias("so ");
	addNew  ("vertex add",       0, "",     "gf.vertexAdd()"         );
	addAlias("vert add");
	addNew  ("vertex add ",     -1, "",     "gf.vertexAdd(%)"        );
	addAlias("vert add ");
	addNew  ("vertex deselect",  0, "",     "gf.vertexDeselect()"    );
	addAlias("vertex desel");
	addAlias("vert deselect");
	addAlias("vert desel");
	addNew  ("vertex move ",    -1, "",    "gf.vertexMove(%)"        );
	addAlias("vert move ");
	addNew  ("vertex next",      0, "",     "gf.vertexNext()"        );
	addAlias("vert next");
	addNew  ("vertex previous",  0, "",     "gf.vertexPrevious()"    );
	addAlias("vertex prev");
	addAlias("vert previous");
	addAlias("vert prev");
	addNew  ("vertex remove",    0, "",     "gf.vertexRemove()"      );
	addAlias("vertex rm");
	addAlias("vert remove");
	addAlias("vert rm");
	addNew  ("vertex select ",   1, "",     "gf.vertexSelect(%)"     );
	addAlias("vertex sel ");
	addAlias("vert select ");
	addAlias("vert sel ");
	addNew  ("write ",           1, "s",    "gf.write(%)"            );
	addAlias("w ");
}
#undef addNew
#undef addAlias

void consoleExecuteCmd(char *cmd) {
	static char *tmp=0;
	char *tmp2, *expr, *ret;
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
			printf("%s\n", evalExprStr, err);
	}

	cmdExecutionLevel--;
}
