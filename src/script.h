// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// script module manages embedded python interpreter

// To make function accessible from python,
// add comment [SCRIPT_NAME: funcName] at the line of function declaration in a header file.
// Function can be then accessed using gf.funcName().
// Script scriptWrappers.pl scans all header files and creates necessary wrapper functions.

#ifndef SCRIPT_H
#define SCRIPT_H

// Module initialization, to be called only once
extern void scriptInit();


// Executes python script, returns true on success
extern bool scriptExecFile(char *path);

// Evaluates python expression, returns its string representation
// Returned value is read-only, valid till next call
extern char *scriptEvalExpr(char *expr);


// Catches exception and returns its string representation, NULL if nothing thrown
// Returned value is read-only, valid till next call
extern char *scriptCatchException();

// Throws exception with given message
extern void scriptThrowException(char *str);


// Finalizes Python interpreter
extern void scriptFinalize();

#endif
