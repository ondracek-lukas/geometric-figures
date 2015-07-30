// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// anim takes care of timed events, like smooth rotations

#ifndef ANIM_H
#define ANIM_H

#include <GL/freeglut.h>


// Minimum delay between frames in ms, can be changed anytime
extern int animFrameDelay;

// Module initialization, to be called only once
extern void animInit(void);

// Rotations:

	// Internal structure representing smooth rotation
	struct animRotation;

	// Angular velocity of the smooth rotation in degrees per second, can be changed anytime
	extern GLfloat animRotSpeed;

	// Creates rotation
	extern struct animRotation *animCreateRot(int axis1, int axis2);

	// Destroys rotation
	extern void animDestroyRot(struct animRotation *rot);

	// Starts rotation
	extern void animStartRot(struct animRotation *rot);

	// Stops rotation
	extern void animStopRot(struct animRotation *rot);

#endif
