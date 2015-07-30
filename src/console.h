// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// console takes care of text communication with user
// This is common interface, consolePrivDraw.h interface should be used only by draw module

#ifndef CONSOLE_H
#define CONSOLE_H

#include <GL/freeglut.h>
#include <stdbool.h>

struct utilStrList;

// Module initialization, to be called only once
extern void consoleInit();


// Prints block from stringsData, returns false if doesn't exist
extern bool consolePrintBlock(char *section, char *name);

// Clears printed block
extern void consoleClearBlock();

// Changes status line, returns whether it was changed (otherwise it was already set)
extern bool consolePrintStatus(char *string);

// Prints multiline string, utilStrList will (or not) be destroyed on clear
extern void consolePrintMultilineAtOnce(struct utilStrList *lines, bool destroy);

// Sets multiline mode
extern void consolePrintMultilineBegin();

// Prints line instead (or below in multiline mode) the previous one
extern void consolePrint(char *string);

// Prints one line with error message
extern void consolePrintErr(char *string);

// Clears printed line(s)
extern void consoleClear();


// Returns whether input console line is open (: was pressed)
extern bool consoleIsOpen();

// Key press events handling on console
extern void consoleKeyPress(char c);
extern void consoleBackspace();
extern void consoleEnter();
extern void consoleUp();
extern void consoleDown();

// Executes console command
extern void consoleExecuteCmd(char *cmd);


// Gets/sets the length of the history
extern int consoleGetHistoryMaxCount();
extern void consoleSetHistoryMaxCount(int maxCount);

// Gets the history
extern struct utilStrList *consoleGetHistory();

#endif
