// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file
//
#include <Python.h>
#include "scriptEvents.h"
#include <stdarg.h>

#include "safe.h"
#include "anim.h"
#include "script.h"
#include "console.h"
#include "convex.h"

#define GIL_ACQUIRE \
	bool gilRelease=!scriptIsGILAcquired(); \
	if (gilRelease) { scriptAcquireGIL(); }
#define GIL_RELEASE \
	if (gilRelease) { scriptReleaseGIL(); }

// -- managing events' callbacks --

struct callbacks {
	PyObject *callback;
	struct callbacks *next;
};

struct scriptEvent {
	char *name;
	char *argsFormat;
	struct callbacks *callbacks;
};

#define event(varName, name, argsFormat) \
	struct scriptEvent varName = {name, argsFormat, NULL}

event(scriptEventsIdle,     "idle",     "()");
event(scriptEventsOpen,     "open",     "()");
event(scriptEventsWrite,    "write",    "(s)");
event(scriptEventsNew,      "new",      "()");
event(scriptEventsModified, "modified", "()");

#undef event

#define event(varName) \
	if (strcmp(varName.name, name)==0) \
		return &varName
struct scriptEvent *eventFromName(char *name) {
	if (!name)
		return NULL;
	event(scriptEventsIdle);
	event(scriptEventsOpen);
	event(scriptEventsWrite);
	event(scriptEventsNew);
	event(scriptEventsModified);
	return NULL;
}
#undef event

PyObject *scriptEventsRegisterCallback(PyObject *self, PyObject *args) {
	char *name;
	PyObject *callback;
	if (!PyArg_ParseTuple(args, "sO", &name, &callback))
		return NULL;
	if (!PyCallable_Check(callback)) {
		PyErr_SetString(PyExc_TypeError, "The second parameter must be callable");
		return NULL;
	}
	Py_INCREF(callback);

	struct scriptEvent *event=eventFromName(name);
	if (!event) {
		scriptThrowException("Wrong callback name");
		return NULL;
	}

	struct callbacks **cb=&event->callbacks;
	while (*cb)
		cb=&(*cb)->next;
	*cb=safeMalloc(sizeof(struct callbacks));
	(*cb)->next=NULL;
	(*cb)->callback=callback;
	return Py_None;
}
PyObject *scriptEventsUnregisterCallback(PyObject *self, PyObject *args) {
	char *name;
	PyObject *callback;
	if (!PyArg_ParseTuple(args, "sO", &name, &callback))
		return NULL;
	if (!PyCallable_Check(callback)) {
		PyErr_SetString(PyExc_TypeError, "The second parameter must be callable");
		return NULL;
	}

	struct scriptEvent *event=eventFromName(name);
	if (!event) {
		scriptThrowException("Wrong callback name");
		return NULL;
	}

	struct callbacks **cb=&event->callbacks;
	while (*cb && ((*cb)->callback != callback))
		cb=&(*cb)->next;
	if (!*cb) {
		scriptThrowException("Callback not found");
		return NULL;
	}

	struct callbacks *item=*cb;
	Py_DECREF(item->callback);
	*cb=item->next;
	free(item);
	return Py_None;
}
#undef event


// -- performing events --

static void performEvent(struct scriptEvent *event, PyObject *args) {
	int count=0;
	struct callbacks *cb;
	int i;
	for (cb=event->callbacks; cb; cb=cb->next)
		count++;
	PyObject *cbs[count];
	for (cb=event->callbacks, i=0; cb; cb=cb->next, i++) {
		cbs[i]=cb->callback;
		Py_INCREF(cbs[i]);
	}
	for (i=0; i<count; i++) {
		PyEval_CallObject(cbs[i], args);
		scriptCatchExceptionAndPrint();
		Py_DECREF(cbs[i]);
	}
}

void scriptEventsPerform(struct scriptEvent *event, ...) {
	GIL_ACQUIRE
	va_list vargs;
	va_start(vargs, event);
	PyObject *args=Py_VaBuildValue(event->argsFormat, vargs);
	if (PyErr_Occurred()) { // broken assertion
		PyErr_Clear();
		printf("Err: Wrong event");
		return;
	}
	performEvent(event, args);
	GIL_RELEASE
}
