// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// convexInteract interacts with user during convex hull generation

#ifndef CONVEX_INTERACT_H
#define CONVEX_INTERACT_H

#include <stdbool.h>

// declared also in convex.h:

	// Determines whether convexInteract is active, read-only
	extern bool convexInteract;

	// Key press event on convexInteract module
	extern void convexInteractKeyPress();

// Determines whether convex hull generation was aborted (e.g. by user)
extern char *convexInteractAborted;


// Starts interaction with given status message
extern void convexInteractStart(char *msg);

// Stops interaction with given message
extern void convexInteractStop(char *msg);

// Updates status message and checks for key press events (if it is not too early)
extern void convexInteractUpdate();

// Aborts convex hull generation with given message
extern void convexInteractAbort(char *msg);

#endif
