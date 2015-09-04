// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// hid manages keyboard and mouse mapping and events

#ifndef HID_H
#define HID_H

#include "anim.h"

// Mouse sensitivity in degrees per pixel, can be changed anytime
float hidMouseSensitivity;

// Determines whether to grab mouse while pressing a button, can be changed anytime
bool hidGrabMouse;


// Module initialization, to be called only once
extern void hidInit();

// Assign given command (NULL to unassign) to the given key code
void hidMap(int code, char *cmd);

// Assign given rotation (NULL to unassign) to the given key code, returns previously assigned rotation
extern struct animRotation *hidMapRot(int code, struct animRotation *rot);

// Returns rotation assigned to the given key code
extern struct animRotation *hidGetMappedRot(int code);


// Event handling: (c is +<glut key code> or -<glut special key code>)

	// Key event (includes mouse buttons)
	extern void hidKeyEvent(int code);

	// Mouse move event, to be called just before button press and while holding it
	extern void hidMouseMoveEvent(int x, int y, int modifiers);

	// Idle event, to be called when nothing is happening
	// Returns whether callback was called
	extern bool hidIdleEvent();

	// Invokes all events waiting from anim sleeping
	extern void hidInvokeWaitingEvents();

// Returns code from keyboard event information
int hidCodeFromEvent(int keyCode, bool isSpecial, int modifiers, bool pressed) __attribute__((const));

// Returns code from mouse button event information
int hidCodeFromMouseEvent(int button, int modifiers, bool pressed);

// Returns code from key/axis name
extern int hidCodeFromString(char *s) __attribute__((pure));


#endif
