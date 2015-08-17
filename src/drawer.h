// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// drawer manages everything displayed, draws figures and controls

#ifndef DRAWER_H
#define DRAWER_H

#include <GL/freeglut.h>
#include <stdbool.h>

// Proportions:

	// Sphere of this radius located in the origin is always visible
	extern const GLfloat drawerVisibleRadius;


	// Dimension of drawing space, read-only (use drawerSetDim)
	extern int drawerDim;

	// Sets dimension of drawing space
	extern void drawerSetDim(int dim);


	// Call drawerSetProjection after change:

		// Distance of the camera from the origin, in (drawerVisibleRadius, +Inf)
		extern GLfloat *drawerCamPos;

		// Diameter of the vertices in px (in the origin), in [drawerEdgeSize, drawerSelectedVertSize]
		extern GLfloat drawerVertSize;

		// Diameter of the edges in px, in (0, drawerVertSize]
		extern GLfloat drawerEdgeSize;

		// Diameter of the selected vertex in px, in [drawerVertSize, +Inf)
		extern GLfloat drawerSelectedVertSize;

	// Sets projection
	extern void drawerSetProjection();


// Colors:

	// Can be changed anytime:

		// Color of the faces
		extern GLfloat drawerFaceColor[4];

		// Space color in origin, positive #th axis, or negative #th axis (see :help set spacecolor)
		extern GLfloat drawerSpaceColorCenter[4];
		extern GLfloat (*drawerSpaceColorPositive)[4];
		extern GLfloat (*drawerSpaceColorNegative)[4];

		// Color of the selected vertex
		extern GLfloat drawerSelectedVertColor[4];


	// Background color, read-only (use drawerSetBackColor)
	extern GLfloat drawerBackColor[4];

	// Sets background color
	extern void drawerSetBackColor(GLfloat *color);


	// Reset all colors to their initial values
	extern void drawerResetColors(); // [SCRIPT_NAME: resetColors]


// Redisplaying

	// Delay between last two finished repainting, read-only
	extern int drawerLastDelay;

	// The total count of redisplays, can overflow, read-only
	extern unsigned drawerRedisplayCounter;

	// Schedules redisplay
	extern void drawerInvokeRedisplay();

	// Returns whether there is a scheduled redisplay
	extern bool drawerWaitingRedisplay();


// Event handling:

	// Module initialization, to be called only once
	extern void drawerInit();

	// Resize event handler
	extern void drawerResize(int w, int h);

	// Display event handler
	extern void drawerDisplay();

	// To be called on exit
	extern void drawerFree();

#endif
