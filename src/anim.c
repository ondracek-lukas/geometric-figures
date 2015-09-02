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

int animFrameDelay=33;           // ms
const int animResponseDelay=100;
GLfloat animRotSpeed=90;         // degrees per second
bool animSleepActive=false;
bool animRedisplayNeeded=false;
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

void frame(int value) {
	char str[100], *str2;
	int time=glutGet(GLUT_ELAPSED_TIME);
	int delay=time-lastTime;
	int rots=0;

	glutTimerFunc(animFrameDelay, frame, 0);
	if (!animSleepActive && sleepInterrupted) {
		sleepInterrupted=false;
		hidInvokeWaitingEvents();
		return;
	}

	lastTime=time;

	if (convexInteract)
		return;

	for (struct animRotation *r=activeRots; r; r=r->next)
		rots+=animCustomRot(r, animRotSpeed*delay/1000);

	str2=str;
	if (rots || animSleepActive) {
		sprintf(str2, "%3d FPS, ", 1000/drawerLastDelay);
		while (*str2) str2++;
	}
	if (rots) {
		sprintf(str2, "%d rotations, ", rots);
		while (*str2) str2++;
	}
	sprintf(str2, "scale %.3f", figureScale);

	if (consolePrintStatus(str) || rots || animRedisplayNeeded) {
		animRedisplayNeeded=false;
		drawerInvokeRedisplay();
	}
}

bool animSleep(int ms) {
	if (sleepInterrupted) {
		return false;
	}
	int time = glutGet(GLUT_ELAPSED_TIME);
	int returnAt = time + ms;
	int wakeAt, wakeAfter;
	int redisplayCounter=drawerRedisplayCounter;
	animSleepActive=true;
	animRedisplayNeeded=true;
	
	while ((redisplayCounter==drawerRedisplayCounter) || ((ms=returnAt-time)>0)) {
		wakeAfter=animFrameDelay/4;
		if (wakeAfter > animResponseDelay)
			wakeAfter=animResponseDelay;
		if (wakeAfter > ms)
			wakeAfter=ms;
		wakeAt=time+wakeAfter;

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
	return !sleepInterrupted;
}

void animSleepInterrupt() {
	sleepInterrupted=true;
}

int animGetTime() {
	return glutGet(GLUT_ELAPSED_TIME);
}
