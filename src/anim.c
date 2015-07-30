// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "anim.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>

#include "keyboard.h"
#include "figure.h"
#include "convex.h"
#include "console.h"
#include "util.h"
#include "safe.h"

int animFrameDelay=33;   // ms
GLfloat animRotSpeed=90; // degrees per second

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

void frame(int value) {
	char *string=0;
	int time=glutGet(GLUT_ELAPSED_TIME);
	int delay=time-lastTime;
	int rots=0;

	lastTime=time;
	glutTimerFunc(animFrameDelay, frame, 0);

	if (convexInteract)
		return;

	for (struct animRotation *r=activeRots; r; r=r->next) {
		if ((r->axis1<figureData.dim) && (r->axis2<figureData.dim)) {
			figureRotate(r->axis1, r->axis2, animRotSpeed*delay/1000);
			rots++;
		} else
			consolePrintErr("Wrong axes");
	}

	utilStrRealloc(&string, 0, 50);
	if (rots)
		sprintf(string, "%3d FPS, %d rotations, scale %.3f", 1000/delay, rots, figureScale);
	else
		sprintf(string, "scale %.3f", figureScale);

	if (consolePrintStatus(string) || rots) {
		glutPostRedisplay();
	}
	utilStrRealloc(&string, 0, 0);
}
