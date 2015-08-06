// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include <Python.h>
#include <stdbool.h>

#include "script.h"
#include "scriptWrappers.h.tmp"

static PyObject *mainModule;
static PyObject *mainDict;

void scriptInit() {
	Py_Initialize();
	mainModule=PyImport_AddModule("__main__");
	Py_INCREF(mainModule);
	mainDict=PyModule_GetDict(mainModule);
	Py_INCREF(mainDict);

	Py_InitModule("gf", scriptWrappersList);
	PyRun_SimpleString("import gf\n");
}

bool scriptExecFile(char *path) {
	PyObject *file=PyFile_FromString(path, "r");
	if (file) {
		if (PyRun_SimpleFileEx(PyFile_AsFile(file), path, 1)!=0) {
			scriptThrowException("Error occurred while executing file, see stderr.");
			return false;
		}
	} else {
		// PyErr_Clear();
		return false;
	}
	return true;
}

// Returns read-only string valid till next call
char *scriptEvalExpr(char *expr) {
	static PyObject *str=NULL;
	Py_XDECREF(str); str=NULL;

	PyObject *obj = PyRun_String(expr, Py_eval_input, mainDict, mainDict);
	if (obj) {
		str = PyObject_Str(obj);
		Py_DECREF(obj);
		return PyString_AsString(str);
	} else {
		return NULL;
	}
}


// Returns read-only string valid till next call
char *scriptCatchException() {
	static PyObject *str=NULL;
	Py_XDECREF(str); str=NULL;

	//PyObject *obj=PyErr_Occurred();
	PyObject *objType, *objValue, *objTraceback;
	PyErr_Fetch(&objType, &objValue, &objTraceback);
	if (objType) {
		if (objValue)
			str=PyObject_Str(objValue);
		else
			str=PyObject_Str(objType);
		Py_XDECREF(objType);
		Py_XDECREF(objValue);
		Py_XDECREF(objTraceback);
		//PyErr_Clear();
		return PyString_AsString(str);
	}
	return NULL;
}

void scriptThrowException(char *str) {
	PyErr_SetString(PyExc_RuntimeError, str);
}


void scriptFinalize() {
	Py_Finalize();
}


