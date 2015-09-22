// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include <GL/freeglut.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "safe.h"
#include "figure.h"
#include "drawer.h"
#include "console.h"
#include "consoleCmd.h"
#include "util.h"
#include "hid.h"
#include "anim.h"
#include "script.h"

static void mainKeyPress(unsigned char c, int x, int y) {
	hidKeyEvent(hidCodeFromEvent(c, false, glutGetModifiers(), true));
}

static void mainSpecialKeyPress(int c, int x, int y) {
	hidKeyEvent(hidCodeFromEvent(c, true, glutGetModifiers(), true));
}

static void mainKeyRelease(unsigned char c, int x, int y) {
	hidKeyEvent(hidCodeFromEvent(c, false, glutGetModifiers(), false));
}

static void mainSpecialKeyRelease(int c, int x, int y) {
	hidKeyEvent(hidCodeFromEvent(c, true, glutGetModifiers(), false));
}

static void mainMouseEvent(int button, int state, int x, int y) {
	int m=glutGetModifiers();
	hidMouseMoveEvent(x, y, m);
	hidKeyEvent(hidCodeFromMouseEvent(button, m, (state==GLUT_DOWN)));
}

static void mainMouseMoveEvent(int x, int y) {
	hidMouseMoveEvent(x, y, glutGetModifiers());
}

int main(int argc, char **argv) {
	char *path;
	FILE *file;

	srand(time(0));
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 768);
	glutInitDisplayMode(GLUT_MULTISAMPLE|GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	glutCreateWindow("Geometric figures");
	hidInit();
	consoleInit();
	figureInit();
	drawerInit();
	animInit();
	scriptInit();


	glutDisplayFunc(drawerDisplay);
	glutReshapeFunc(drawerResize);
	glutKeyboardFunc(mainKeyPress);
	glutSpecialFunc(mainSpecialKeyPress);
	glutKeyboardUpFunc(mainKeyRelease);
	glutSpecialUpFunc(mainSpecialKeyRelease);
	glutMouseFunc(mainMouseEvent);
	glutMotionFunc(mainMouseMoveEvent);

	path=utilExpandPath("%/config.py");
	if (file=fopen(path, "r")) {
		fclose(file);
		consoleExecFile(path);
	}

	consolePrintNamedBlock("help", "welcome");

	glutMainLoop();

	drawerFree();
	scriptFinalize();

	return 0;
}
