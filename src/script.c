// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include <Python.h>
#include <stdbool.h>

#include "script.h"
#include "scriptWrappers.h.tmp"
#include "safe.h"

static PyObject *mainModule;
static PyObject *mainDict;
static PyThreadState *threadState;

#define GIL_ACQUIRE \
	bool gilRelease=threadState; \
	if (gilRelease) {PyEval_RestoreThread(threadState); threadState=NULL; }
#define GIL_RELEASE \
	if (gilRelease) {threadState=PyEval_SaveThread(); }

void scriptAcquireGIL() {
	if (!threadState)
		safeExitErr("GIL already acquired\n");
	PyEval_RestoreThread(threadState);
	threadState=NULL;
}
void scriptReleaseGIL() {
	threadState=PyEval_SaveThread();
}

void scriptInit() {
	PyEval_InitThreads();
	Py_Initialize();
	mainModule=PyImport_AddModule("__main__");
	Py_INCREF(mainModule);
	mainDict=PyModule_GetDict(mainModule);
	Py_INCREF(mainDict);

	Py_InitModule("gf", scriptWrappersList);
	PyRun_SimpleString("import gf\n");

	threadState=PyEval_SaveThread();
}

bool scriptExecFile(char *path) {
	GIL_ACQUIRE
	PyObject *file=PyFile_FromString(path, "r");
	if (file) {
		if (PyRun_SimpleFileEx(PyFile_AsFile(file), path, 1)!=0) {
			scriptThrowException("Error occurred while executing file, see stderr.");
			GIL_RELEASE
			return false;
		}
	} else {
		// PyErr_Clear();
		GIL_RELEASE
		return false;
	}
	GIL_RELEASE
	return true;
}

// Returns read-only string valid till next call
char *scriptEvalExpr(char *expr) {
	GIL_ACQUIRE
	static PyObject *str=NULL;
	Py_XDECREF(str); str=NULL;

	PyObject *obj = PyRun_String(expr, Py_eval_input, mainDict, mainDict);
	if (obj) {
		if (obj==Py_None) {
			GIL_RELEASE
			return "";
		} else {
			str = PyObject_Str(obj);
			Py_DECREF(obj);
			char *ret=PyString_AsString(str);
			GIL_RELEASE
			return ret;
		}
	} else {
		GIL_RELEASE
		return NULL;
	}
}


// Returns read-only string valid till next call
char *scriptCatchException() {
	GIL_ACQUIRE
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
		char *ret=PyString_AsString(str);
		GIL_RELEASE
		return ret;
	}
	GIL_RELEASE
	return NULL;
}

void scriptThrowException(char *str) {
	GIL_ACQUIRE
	PyErr_SetString(PyExc_RuntimeError, str);
	GIL_RELEASE
}


void scriptFinalize() {
	PyEval_RestoreThread(threadState);
	Py_Finalize();
}


