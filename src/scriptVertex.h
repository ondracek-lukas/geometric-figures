// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// scriptVertex module allows managing vertices from Python

// Adds new vertex returning its index, optionally takes tuple with coordinates
extern PyObject *scriptVertexAdd(PyObject *self, PyObject *args);              // [SCRIPT_NAME: vertexAdd]

// Sets vertex position, takes index of the vertex and tuple with new coordinates
extern PyObject *scriptVertexSetPos(PyObject *self, PyObject *args);           // [SCRIPT_NAME: vertexSetPos]

// Returns tuple with coordinates of a vertex, takes index of the vertex
extern PyObject *scriptVertexGetPos(PyObject *self, PyObject *args);           // [SCRIPT_NAME: vertexGetPos]

// Removes vertex with the given index
extern PyObject *scriptVertexRm(PyObject *self, PyObject *args);               // [SCRIPT_NAME: vertexRemove]


// Takes tuple with coordinates, returns them transformed according to the current figure rotation
extern PyObject *scriptVertexRotatePoint(PyObject *self, PyObject *args);      // [SCRIPT_NAME: posRotate]

// Inverse of the previous function
extern PyObject *scriptVertexRotatePointBack(PyObject *self, PyObject *args);  // [SCRIPT_NAME: posRotateBack]
