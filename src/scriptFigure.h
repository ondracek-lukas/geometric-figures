// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// scriptFigure module allows managing figures from Python

// To find out the Python format of the figure, see :help python figure

#ifndef SCRIPT_FIGURE_H
#define SCRIPT_FIGURE_H

#ifdef Py_PYTHON_H
// Returns currently opened figure
extern PyObject *scriptFigureGet(PyObject *self, PyObject *args);   // [SCRIPT_NAME: figureGet]

// Opens given figure
extern PyObject *scriptFigureOpen(PyObject *self, PyObject *args);  // [SCRIPT_NAME: figureOpen]

// Updates convex hull of the given figure
extern PyObject *scriptFigureConvexHullUpdate(PyObject *self, PyObject *args); // [SCRIPT_NAME: figureConvexHullUpdate]

#endif

// Converts figure to Python expression
char *scriptFigureToPythonExpr(struct figureData *figure);

#endif
