// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// debug module contains useful debugging functions and macros

// This header file can be used to enable or disable debugging of specific parts of code.
// It has to be included in all modules which use it,
// but the module itself has not to be linked if debugging is not enabled.

// All temporary code (used for debugging) in other modules should be written in DEBUG_*, e.g.:
	// DEBUG_MY_FEATURE(printf("Everything went wrong...\n"); exit(42);)

#ifndef DEBUG_H
#define DEBUG_H

// #define DEBUG

#define DEBUG_EXEC(expr) expr
#define DEBUG_RM(expr)

#define DEBUG_HULL         DEBUG_RM
#define DEBUG_HULL_VERBOSE DEBUG_RM
#define DEBUG_HULL_DOT     DEBUG_RM
#define DEBUG_LOOP         DEBUG_RM
#define DEBUG_CMDS         DEBUG_RM


#ifdef DEBUG
#include <GL/freeglut.h>

struct convexFigBst;
struct convexFigList;

extern void convexFigPrint(); // mark unsafe
extern void convexFigPrintR(struct convexFigList *list, char *parent);
extern void convexFigBstPrint(struct convexFigBst *bst);
extern void convexFigBstPrintR(struct convexFigBst *bst, char *parent, char *attr);
extern int convexFigBstCheck(struct convexFigBst *bst, char *str);
extern int convexFigBstCheckR(struct convexFigBst *bst, GLfloat *min, GLfloat *max, int *depth, char *str);
extern void convexLoopDetectPrint(int count, int parent, int child, int index, int next);
#endif

#endif
