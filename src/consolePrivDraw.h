// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// This interface should be used only by draw module, read-only
// Common interface is console.h
// console takes care of text communication with user

#ifndef CONSOLE_PRIV_DRAW_H
#define CONSOLE_PRIV_DRAW_H

extern int consoleInLength;
extern char *consoleStatus;
extern struct utilStrList *consoleLines;
extern struct utilStrList *consoleBlock;
extern int consoleBlockWidth;
extern int consoleBlockHeight;

// defined in consoleCmdVertex.c
	extern int consoleCmdVertexSelected;

#endif
