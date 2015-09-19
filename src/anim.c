// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "anim.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "hid.h"
#include "figure.h"
#include "convex.h"
#include "console.h"
#include "util.h"
#include "safe.h"
#include "drawer.h"
#include "script.h"
#include "scriptEvents.h"

int animFrameDelay=33;           // ms
const int animResponseDelay=100;
GLfloat animRotSpeed=90;         // degrees per second
bool animSleepActive=false;
static bool sleepInterrupted=false;

struct animRotation {
	int axis1, axis2;
	struct animRotation *next;
};

static int lastTime;
static void frame(int value);
static struct animRotation *activeRots=0, **activeRotsEnd=&activeRots, *freedRots=0;

void animInit() {
	lastTime=glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc(animFrameDelay, frame, 0);
}


struct animRotation *animCreateRot(int axis1, int axis2) {
	struct animRotation *rot;
	if (freedRots) {
		rot=freedRots;
		freedRots=rot->next;
	} else {
		rot=safeMalloc(sizeof(struct animRotation));
	}
	rot->axis1=axis1;
	rot->axis2=axis2;
	return rot;
}

void animDestroyRot(struct animRotation *rot) {
	if (!rot)
		return;
	animStopRot(rot);
	rot->next=freedRots;
	freedRots=rot;
}

void animStartRot(struct animRotation *rot) {
	animStopRot(rot);
	*activeRotsEnd=rot;
	activeRotsEnd=&rot->next;
}

void animStopRot(struct animRotation *rot) {
	struct animRotation **r=&activeRots;
	while (*r) {
		if (*r==rot)
			*r=(*r)->next;
		else
			r=&(*r)->next;
	}
	activeRotsEnd=r;
	rot->next=0;
}

bool animCustomRot(struct animRotation *rot, GLfloat angle) {
	if ((rot->axis1<figureData.dim) && (rot->axis2<figureData.dim)) {
		figureRotate(rot->axis1, rot->axis2, angle);
		return true;
	} else {
		consolePrintErr("Wrong axes");
		return false;
	}
}

void updateStatus(int rotations, bool showFps, char *msg) {
	char str[100], *str2;
	str2=str;
	if (rotations) {
		sprintf(str2, "%d rotations, ", rotations);
		while (*str2) str2++;
	}
	if (msg) {
		str2=stpcpy(str2, msg);
		str2=stpcpy(str2, ", ");
	}
	if (showFps) {
		int redisplayDelay=glutGet(GLUT_ELAPSED_TIME)-drawerLastRedisplayTime;
		if (redisplayDelay<drawerLastDelay)
			redisplayDelay=drawerLastDelay;
		if (redisplayDelay<=1000) {
			sprintf(str2, "%2d FPS, ", 1000/redisplayDelay);
			while (*str2) str2++;
		}
	}
	sprintf(str2, "scale %.3f", figureScale);
	consolePrintStatus(str);
}

void frame(int value) {
	int time=glutGet(GLUT_ELAPSED_TIME);
	int delay=time-lastTime;
	int rots=0;

	lastTime=time;
	glutTimerFunc(animFrameDelay, frame, 0);
	if (animSleepActive) {
		return;
	} else if (!animSleepActive && sleepInterrupted) {
		sleepInterrupted=false;
		hidInvokeWaitingEvents();
		return;
	}

	if (convexInteract)
		return;

	for (struct animRotation *r=activeRots; r; r=r->next)
		rots+=animCustomRot(r, animRotSpeed*delay/1000);

	if (!rots)
		scriptEventsSchedulePending();

	updateStatus(rots, true, 0);
}

bool animSleep(int ms) {
	if (sleepInterrupted) {
		return false;
	}
	scriptReleaseGIL();
	int time = glutGet(GLUT_ELAPSED_TIME);
	int returnAt = time + ms;
	int wakeAt, wakeAfter;
	animSleepActive=true;

	while ((ms=returnAt-time)>0) {
		wakeAfter=animFrameDelay;
		if (wakeAfter > animResponseDelay)
			wakeAfter=animResponseDelay;
		if (wakeAfter > ms)
			wakeAfter=ms;
		wakeAt=time+wakeAfter;

		updateStatus(0, true, "script");
		glutMainLoopEvent();
		if (sleepInterrupted)
			break;

		time = glutGet(GLUT_ELAPSED_TIME);
		wakeAfter=wakeAt-time;
		if (wakeAfter>0) {
			utilSleep(wakeAfter);
		}
		time = glutGet(GLUT_ELAPSED_TIME);
	}
	glutMainLoopEvent();
	animSleepActive=false;
	scriptAcquireGIL();
	scriptEventsSchedulePending();
	return !sleepInterrupted;
}

void animSleepInterrupt() {
	sleepInterrupted=true;
}

int animGetTime() {
	return glutGet(GLUT_ELAPSED_TIME);
}
