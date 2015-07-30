// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// keyboard manages key mapping and events

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "anim.h"


// Module initialization, to be called only once
extern void keyboardInit();

// Assign given command (NULL to unassign) to the given key code
void keyboardMap(int code, char *cmd);

// Assign given rotation (NULL to unassign) to the given key code, returns previously assigned rotation
extern struct animRotation *keyboardMapRot(int code, struct animRotation *rot);

// Returns rotation assigned to the given key code
extern struct animRotation *keyboardGetMappedRot(int code);


// Event handling: (c is +<glut key code> or -<glut special key code>)

	// Keyboard press
	extern void keyboardPress(int c);

	// Keyboard release
	extern void keyboardRelease(int c);


// Returns key code from string name
extern int keyboardCodeFromString(char *s);


#endif
