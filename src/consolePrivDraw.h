// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// This interface should be used only by draw module, read-only
// Common interface is console.h
// console takes care of text communication with user

#ifndef CONSOLE_PRIV_DRAW_H
#define CONSOLE_PRIV_DRAW_H

extern int consoleInLength;
extern char *consoleIn;
extern char *consoleOut;
extern char *consoleStatus;
extern struct utilStrList *consoleBlock;
extern int consoleBlockWidth;
extern int consoleBlockHeight;
extern struct utilStrList *consolePrintMultilineList;
extern enum consolePrintMode {
	consolePrintOneLine,
	consolePrintOneLineErr,
	consolePrintMultiline
} consolePrintMode;

// defined in consoleCmdVertex.c
	extern int consoleCmdVertexSelected;

#endif
