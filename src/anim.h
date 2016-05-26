// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// anim takes care of timed events, like smooth rotations

#ifndef ANIM_H
#define ANIM_H

#include <GL/freeglut.h>
#include <stdbool.h>


// Minimum delay between frames in ms, can be changed anytime
extern int animFrameDelay;

// Maximum delay of response in ms
extern const int animResponseDelay;

// Determines whether animSleep is active, read-only
extern bool animSleepActive;

// Module initialization, to be called only once
extern void animInit(void);

// Rotations:

	// Internal structure representing smooth rotation
	struct animRotation;

	// Angular velocity of the smooth rotation in degrees per second, can be changed anytime
	extern GLdouble animRotSpeed;

	// Creates rotation
	extern struct animRotation *animCreateRot(int axis1, int axis2);

	// Destroys rotation
	extern void animDestroyRot(struct animRotation *rot);

	// Starts rotation
	extern void animStartRot(struct animRotation *rot);

	// Stops rotation
	extern void animStopRot(struct animRotation *rot);

	// Applies rotation by given angle, returns true on success
	extern bool animCustomRot(struct animRotation *rot, GLdouble angle);

// Determines whether redisplay is needed in next frame, can be set (not cleared) anytime
extern bool animRedisplayNeeded;

// Handles events for given time, then returns
extern bool animSleep(int ms); // [SCRIPT_NAME: sleep]

// Interrupts sleeping, events waiting in hid module will be invoked
extern void animSleepInterrupt();

// Clears interruptions, to be called after performing all waiting events in hid module
extern void animSleepClearInterruption();

// Returns time in ms from the start of app
int animGetTime(); // [SCRIPT_NAME: time]

#endif
