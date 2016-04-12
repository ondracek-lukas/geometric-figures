// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "convexInteract.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "util.h"
#include "console.h"
#include "convex.h"
#include "convexFig.h"
#include "convexSpace.h"
#include "drawer.h"

bool convexInteract=0;
char *convexInteractAborted=0;

static int updateTime=0;
static char *message=0;
static char *messageStatus;

static void updateStatus();

void convexInteractStart(char *msg) {
	if (!msg)
		msg="";
	convexInteractAborted=0;
	convexInteract=1;
	utilStrRealloc(&message, 0, strlen(msg)+1);
	strcpy(message, msg);
	messageStatus=strchr(message, '\0');
	consoleClear();
	updateTime=glutGet(GLUT_ELAPSED_TIME);
}

void convexInteractStop(char *msg) {
	if (convexInteractAborted) {
		convexHull=0;
		utilStrRealloc(&message, 0, strlen(convexInteractAborted)+1);
		strcpy(message, convexInteractAborted);
		messageStatus=strchr(message, '\0');
		updateStatus();
		consolePrintErr(message);
	} else if (msg) {
		utilStrRealloc(&message, 0, strlen(msg)+1);
		strcpy(message, msg);
		messageStatus=strchr(message, '\0');
		updateStatus();
		consolePrint(message);
	}
	convexInteract=0;
}

void updateStatus() {
	char *str=messageStatus;
	utilStrRealloc(&message, &str, 8);
	utilStrReallocPtrUpdate(&messageStatus);
	sprintf(str, " (");
	str=strchr(str, '\0');

	utilStrRealloc(&message, &str,
		ceil(log(abs(convexFigCount)+1)/log(10))+20);
	utilStrReallocPtrUpdate(&messageStatus);

	sprintf(str, "%d faces in total)", convexFigCount);
	str=strchr(str, '\0');
}

void convexInteractUpdate() {
	if (!convexInteract)
		return;
	if (glutGet(GLUT_ELAPSED_TIME)-updateTime<200)
		return;
	if (*message!='\0')
		updateStatus();
	consolePrintStatus(message);
	drawerInvokeRedisplay();
	glutMainLoopEvent();
	updateTime=glutGet(GLUT_ELAPSED_TIME);
}

void convexInteractAbort(char *msg) {
	utilStrRealloc(&message, 0, 16);
	strcpy(message, "Aborting...");
	messageStatus=strchr(message, '\0');
	convexInteractAborted=msg;
}

void convexInteractKeyPress() {
	convexInteractAbort("Boundary update was aborted by user");
}
