// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// scriptFigure module allows managing figures from Python

// To find out the Python format of the figure, see :help python figure

// Returns currently opened figure
extern PyObject *scriptFigureGet(PyObject *self, PyObject *args);   // [SCRIPT_NAME: figureGet]

// Opens given figure
extern PyObject *scriptFigureOpen(PyObject *self, PyObject *args);  // [SCRIPT_NAME: figureOpen]

// Reads figure from a file and returns it, takes path of the file
extern PyObject *scriptFigureRead(PyObject *self, PyObject *args);  // [SCRIPT_NAME: figureRead]

// Writes given figure to a file, takes path of the file and the figure
extern PyObject *scriptFigureWrite(PyObject *self, PyObject *args); // [SCRIPT_NAME: figureWrite]
