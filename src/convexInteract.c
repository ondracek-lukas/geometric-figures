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
	int maxdim=0, dim=0, count=0;
	struct convexFigList *figures=0, *list;
	utilStrRealloc(&message, &str, 8);
	utilStrReallocPtrUpdate(&messageStatus);
	sprintf(str, " (");
	str=strchr(str, '\0');
	convexFigBstGetAll(convexSpaces, &figures);
	for (list=figures; list; list=list->next)
		if (list->fig->space->dim==maxdim)
			count++;
		else if (list->fig->space->dim>maxdim) {
			maxdim=list->fig->space->dim;
			count=1;
		}

	utilStrRealloc(&message, &str,
		(ceil(log(convexFigListLen(figures))/log(10))+20)*maxdim);
	utilStrReallocPtrUpdate(&messageStatus);
	if (maxdim>0) {
		sprintf(str, "%d figures, ", count);
		str=strchr(str, '\0');
		dim=maxdim-1;
	}
	for (; dim>=0; dim--) {
		if (dim<maxdim) {
			count=0;
			for (list=figures; list; list=list->next)
				if (list->fig->space->dim==dim)
					count++;
		}
		if (dim>=4)
			sprintf(str, "%d %d-faces, ", count, dim);
		else if (dim==3)
			sprintf(str, "%d cells, ", count);
		else if (dim==2)
			sprintf(str, "%d faces, ", count);
		else if (dim==1)
			sprintf(str, "%d edges, ", count);
		else
			sprintf(str, "%d vertices)", count);
		str=strchr(str, '\0');
	}
	convexFigListDestroy(&figures);
}

void convexInteractUpdate() {
	if (glutGet(GLUT_ELAPSED_TIME)-updateTime<100)
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
