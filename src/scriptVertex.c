// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include <Python.h>

#include <stdbool.h>
#include "script.h"
#include "figure.h"
#include "safe.h"
#include "matrix.h"

static bool coordsFromPython(PyObject *pyCoords, GLfloat *coords);
static PyObject *coordsToPython(GLfloat *coords);


PyObject *scriptVertexAdd(PyObject *self, PyObject *args) {
	PyObject *pyCoords=Py_None;
	if (!PyArg_ParseTuple(args, "|O", &pyCoords))
		return NULL;

	if (figureData.dim<0) {
		scriptThrowException("There is no space yet, use new or open");
		return NULL;
	}

	GLfloat coords[figureData.dim];
	if (!coordsFromPython(pyCoords, coords))
		return NULL;

	if (!safeCheckPos(coords, figureData.dim)) {
		scriptThrowException("Wrong position");
		return NULL;
	}

	return Py_BuildValue("i", figureVertexAdd(coords));
}

PyObject *scriptVertexSetPos(PyObject *self, PyObject *args) {
	PyObject *pyCoords;
	int index;
	if (!PyArg_ParseTuple(args, "iO", &index, &pyCoords))
		return NULL;

	if ((figureData.dim<0) || (index<0) || (index>=figureData.count[0])) {
		scriptThrowException("Out of bounds");
		return NULL;
	}

	GLfloat coords[figureData.dim];
	if (!coordsFromPython(pyCoords, coords))
		return NULL;

	if (!safeCheckPos(coords, figureData.dim)) {
		scriptThrowException("Wrong position");
		return NULL;
	}

	figureVertexMove(index, coords);
	return Py_None;
}


PyObject *scriptVertexGetPos(PyObject *self, PyObject *args) {
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	if ((figureData.dim<0) || (index<0) || (index>=figureData.count[0])) {
		scriptThrowException("Out of bounds");
		return NULL;
	}

	return coordsToPython(figureData.vertices[index]);
}

PyObject *scriptVertexRm(PyObject *self, PyObject *args) {
	int index;
	if (!PyArg_ParseTuple(args, "i", &index))
		return NULL;

	if ((figureData.dim<0) || (index<0) || (index>=figureData.count[0])) {
		scriptThrowException("Out of bounds");
		return NULL;
	}

	figureVertexRm(index);

	return Py_None;
}


// -- coordinates manipulation --

PyObject *scriptVertexRotatePoint(PyObject *self, PyObject *args) {
	PyObject *pyCoords;
	GLfloat coords[figureData.dim], coords2[figureData.dim];
	if (!PyArg_ParseTuple(args, "O", &pyCoords))
		return NULL;
	if (!coordsFromPython(pyCoords, coords))
		return NULL;

	matrixProduct(figureRotMatrix, coords, coords2, figureData.dim, figureData.dim, 1);
	return coordsToPython(coords2);
}

PyObject *scriptVertexRotatePointBack(PyObject *self, PyObject *args) {
	PyObject *pyCoords;
	GLfloat coords[figureData.dim], coords2[figureData.dim];
	if (!PyArg_ParseTuple(args, "O", &pyCoords))
		return NULL;
	if (!coordsFromPython(pyCoords, coords))
		return NULL;

	matrixProduct(coords, figureRotMatrix, coords2, 1, figureData.dim, figureData.dim);
	return coordsToPython(coords2);
}


// -- translating coordinates --

bool coordsFromPython(PyObject *pyCoords, GLfloat *coords) {
	if (pyCoords==Py_None) {
		for (int i=0; i<figureData.dim; i++)
			coords[i]=0;
	} else {
		if ((PyTuple_Size(pyCoords)!=figureData.dim) || (PyErr_Occurred())) {
			scriptThrowException("Wrong coordinates");
			return false;
		}
		for (int j=0; j<figureData.dim; j++) {
			PyObject *value=PyTuple_GET_ITEM(pyCoords, j);
			coords[j]=PyFloat_AsDouble(value);
		}
	}
	return true;
}

PyObject *coordsToPython(GLfloat *coords) {
	PyObject *pyCoords=PyTuple_New(figureData.dim);
	for (int i=0; i<figureData.dim; i++)
		PyTuple_SET_ITEM(pyCoords, i, PyFloat_FromDouble(coords[i]));
	return pyCoords;
}
