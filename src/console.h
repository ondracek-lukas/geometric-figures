// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// console takes care of text communication with user
// This is common interface, consolePrivDraw.h interface should be used only by draw module

#ifndef CONSOLE_H
#define CONSOLE_H

#include <GL/freeglut.h>
#include <stdbool.h>

struct utilStrList;

// Determines whether user can type also python expressions instead of commands
// Can be changed anytime.
extern bool consoleAllowPythonExpr;

// Determines whether typed (or translated) python expressions should be printed to stdout
// Can be changed anytime.
extern bool consolePythonExprToStdout;


// Module initialization, to be called only once
extern void consoleInit();


enum consoleSpecialChars { // Special: 3,4,5,6,7,\b; Free: 9
	consoleSpecialBack='\b',
	consoleSpecialColorNormal=3,
	consoleSpecialColorRed=4,
	consoleSpecialColorGreen=5,
	consoleSpecialColorBlue=6,
	consoleSpecialColorGray=7
};

// Returns the length of the longest line of str
// consoleSpecialChars are reflected
int consoleStrWidth(char *str);

// Returns number of lines of str
int consoleStrHeight(char *str);


// Prints block from stringsData, returns false if doesn't exist
extern bool consolePrintBlock(char *section, char *name);

// Clears printed block
extern void consoleClearBlock();

// Changes status line, returns whether it was changed (otherwise it was already set)
extern bool consolePrintStatus(char *string);

// Prints list of lines
extern void consolePrintLinesList(struct utilStrList *lines);

// Sets multiline mode
extern void consolePrintMultilineBegin();

// Prints line instead (or below in multiline mode) the previous one
extern void consolePrint(char *string); // [SCRIPT_NAME: echo]

// Prints one line with error message
extern void consolePrintErr(char *string);

// Clears printed line(s)
extern void consoleClear(); // [SCRIPT_NAME: clear]


// Printing will only append lines to already printed ones
void consoleAppendMode();

// Printing or opening console will clear printed lines
extern void consoleClearBeforePrinting();

// Appends msg line and clears only it while typing command
extern void consoleClearAfterCmd(char *msg);  // [SCRIPT_NAME: clearAfterCmd]
extern void consoleClearAfterCmdDefaultMsg(); // [SCRIPT_NAME: clearAfterCmd]


// Returns whether input console line is open (: was pressed)
extern bool consoleIsOpen();

// Key press events handling on console
extern void consoleKeyPress(char c);
extern void consoleBackspace();
extern void consoleEnter();
extern void consoleUp();
extern void consoleDown();
extern void consoleLeft();
extern void consoleRight();
extern void consoleTab();

// Invokes executing command
extern void consoleExecuteCmd(char *cmd);

// Invokes evaluating python expression
extern void consoleEvalExpr(char *expr);

// Invokes executing python script file
extern void consoleExecFile(char *path);

// Gets/sets the length of the history
extern int consoleGetHistoryMaxCount();
extern void consoleSetHistoryMaxCount(int maxCount);

// Gets the history
extern struct utilStrList *consoleGetHistory();

#endif
