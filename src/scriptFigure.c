// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include <Python.h>

#include <stdbool.h>

#include "figure.h"
#include "script.h"
#include "safe.h"
#include "drawer.h"
#include "util.h"


static struct figureData *figureFromPython(PyObject *pyFigure);
static PyObject *figureToPython(struct figureData *pyFigure);


PyObject *scriptFigureGet(PyObject *self, PyObject *args) {
	if (!PyArg_ParseTuple(args, ""))
		return NULL;
	return figureToPython(&figureData);
}

PyObject *scriptFigureOpen(PyObject *self, PyObject *args) {
	PyObject *pyFigure;
	int preserveRotation=false;
	if (!PyArg_ParseTuple(args, "O|i", &pyFigure, &preserveRotation))
		return NULL;

	struct figureData *figure=figureFromPython(pyFigure);
	if (figure) {
		if (preserveRotation && (figure->dim==-1)) {
			figure->dim=figureData.dim;
			figure->count=safeCalloc(figure->dim+1, sizeof(int));
		}
		figureOpen(figure, preserveRotation);
	}
	if (PyErr_Occurred())
		return NULL;
	return Py_None;
}

PyObject *scriptFigureRead(PyObject *self, PyObject *args) {
	char *path;
	if (!PyArg_ParseTuple(args, "s", &path))
		return NULL;
	struct figureData *figure=figureRead(utilExpandPath(path));
	if (!figure) {
		scriptThrowException("File cannot be opened or has wrong format");
		return NULL;
	}
	PyObject *pyFigure=figureToPython(figure);
	figureDestroy(figure, true);
	return pyFigure;
}

PyObject *scriptFigureWrite(PyObject *self, PyObject *args) {
	char *path;
	PyObject *pyFigure;
	if (!PyArg_ParseTuple(args, "sO", &path, &pyFigure))
		return NULL;
	struct figureData *figure=figureFromPython(pyFigure);
	if (!figure)
		return NULL;
	if (!figureWrite(utilExpandPath(path), figure, false)) {
		figureDestroy(figure, true);
		scriptThrowException("File cannot be opened for writing");
		return NULL;
	}
	figureDestroy(figure, true);
	return Py_None;
}



// -- translating figures --

#define throw(msg) {scriptThrowException(msg); figureDestroy(figure, true); return NULL; }
struct figureData *figureFromPython(PyObject *pyFigure) {
	struct figureData *figure=safeCalloc(1, sizeof(struct figureData));

	if (pyFigure==Py_None) {
		figure->dim=-1;
		return figure;
	}
	figure->dim=PyList_Size(pyFigure)-1;

	if (PyErr_Occurred())
		throw("Wrong figure");
	figure->count=safeMalloc(sizeof(GLint) * (figure->dim+1));

	PyObject *vertices=PyList_GET_ITEM(pyFigure, 0);
	figure->count[0]=PyList_Size(vertices);
	if (PyErr_Occurred())
		throw("Wrong list of vertices");
	figure->vertices=safeCalloc(figure->count[0], sizeof(GLfloat *));
	for (int i=0; i<figure->count[0]; i++) {
		PyObject *vertex=PyList_GET_ITEM(vertices, i);
		figure->vertices[i]=safeMalloc(sizeof(GLfloat) * figure->dim);
		if ((PyTuple_Size(vertex)!=figure->dim) || (PyErr_Occurred()))
			throw("Wrong list of vertices");
		for (int j=0; j<figure->dim; j++) {
			PyObject *value=PyTuple_GET_ITEM(vertex, j);
			figure->vertices[i][j]=PyFloat_AsDouble(value);
		}
		if (!safeCheckPos(figure->vertices[i], figure->dim))
			throw("Wrong vertex position")
	}
	if (PyErr_Occurred())
		throw("Wrong list of vertices");

	figure->boundary=safeCalloc(figure->dim+1, sizeof(GLint **));
	for (int i=1; i<=figure->dim; i++) {
		PyObject *faces=PyList_GET_ITEM(pyFigure, i);
		figure->count[i]=PyList_Size(faces);
		if (PyErr_Occurred())
			throw("Wrong topology");
		figure->boundary[i]=safeCalloc(figure->count[i], sizeof(GLint *));
		for (int j=0; j<figure->count[i]; j++) {
			PyObject *face=PyList_GET_ITEM(faces, j);
			int count=PyList_Size(face);
			if (PyErr_Occurred())
				throw("Wrong topology");
			figure->boundary[i][j]=safeMalloc(sizeof(GLint) * (count+1));
			figure->boundary[i][j][0]=count;
			for (int k=1; k<=count; k++) {
				PyObject *value=PyList_GET_ITEM(face, k-1);
				figure->boundary[i][j][k]=PyInt_AsLong(value);
			}
		}
	}
	if (PyErr_Occurred())
		throw("Wrong topology");

	return figure;
}
#undef throw
PyObject *figureToPython(struct figureData *figure) {
	if (figure->dim<0)
		return Py_None;
	PyObject *pyFigure=PyList_New(figure->dim+1);

	PyObject *vertices=PyList_New(figure->count[0]);
	for (int i=0; i<figure->count[0]; i++) {
		PyObject *vertex=PyTuple_New(figure->dim);
		for (int j=0; j<figure->dim; j++) {
			PyObject *value=PyFloat_FromDouble(figure->vertices[i][j]);
			PyTuple_SET_ITEM(vertex, j, value);
		}
		PyList_SET_ITEM(vertices, i, vertex);
	}
	PyList_SET_ITEM(pyFigure, 0, vertices);

	for (int i=1; i<=figure->dim; i++) {
		PyObject *faces=PyList_New(figure->count[i]);
		for (int j=0; j<figure->count[i]; j++) {
			PyObject *face=PyList_New(figure->boundary[i][j][0]);
			for (int k=1; k<=figure->boundary[i][j][0]; k++) {
				PyObject *value=PyInt_FromLong(figure->boundary[i][j][k]);
				PyList_SET_ITEM(face, k-1, value);
			}
			PyList_SET_ITEM(faces, j, face);
		}
		PyList_SET_ITEM(pyFigure, i, faces);
	}

	return pyFigure;
}
