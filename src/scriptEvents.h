// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// scriptEvents module allows hadling several events from Python

#ifndef SCRIPT_EVENTS_H
#define SCRIPT_EVENTS_H
struct scriptEvent;

// Available events:
extern struct scriptEvent
	scriptEventsIdle,      // "idle";      no arguments
	scriptEventsOpen,      // "open";      char *path
	scriptEventsWrite,     // "write";     char *path
	scriptEventsNew,       // "new";       no arguments
	scriptEventsModified;  // "modified";  no arguments

// Performs event
extern void scriptEventsPerform(struct scriptEvent *event, ...);

#ifdef Py_PYTHON_H
// Registers new callback, takes callback name and callable function
extern PyObject *scriptEventsRegisterCallback(PyObject *self, PyObject *args);   // [SCRIPT_NAME: registerCallback]

// Unregisters callback when called with the same arguments as when registering
extern PyObject *scriptEventsUnregisterCallback(PyObject *self, PyObject *args); // [SCRIPT_NAME: unregisterCallback]
#endif
#endif
