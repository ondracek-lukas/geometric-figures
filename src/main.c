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
#include "keyboard.h"
#include "anim.h"

static void mainKeyPress(unsigned char c, int x, int y) {
	keyboardPress(c);
}

static void mainSpecialKeyPress(int c, int x, int y) {
	keyboardPress(-c);
}

static void mainKeyRelease(unsigned char c, int x, int y) {
	keyboardRelease(c);
}

static void mainSpecialKeyRelease(int c, int x, int y) {
	keyboardRelease(-c);
}

int main(int argc, char **argv) {
	char *path;
	FILE *file;

	srand(time(0));
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 768);
	glutInitDisplayMode(GLUT_MULTISAMPLE|GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
	glutCreateWindow("Geometric figures");
	keyboardInit();
	consoleInit();
	figureInit();
	drawerInit();
	animInit();


	glutDisplayFunc(drawerDisplay);
	glutReshapeFunc(drawerResize);
	glutKeyboardFunc(mainKeyPress);
	glutSpecialFunc(mainSpecialKeyPress);
	glutKeyboardUpFunc(mainKeyRelease);
	glutSpecialUpFunc(mainSpecialKeyRelease);

	path=utilExecutablePath();
	path=strcpy(safeMalloc(sizeof(char)*strlen(path)+6), path);
#ifdef WIN32
	char *extension=strrchr(path, '.');
	if (extension && (strcasecmp(extension, ".exe")==0))
		*extension='\0';
#endif
	strcat(path, ".conf");
	if (file=fopen(path, "r")) {
		fclose(file);
		consoleCmdSource(path);
	}
	free(path);

	consolePrintBlock("help", "welcome");

	glutMainLoop();

	drawerFree();
	return 0;
}
