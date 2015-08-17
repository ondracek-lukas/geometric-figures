// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmd.h"

#include <limits.h>
#include <GL/freeglut.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "console.h"
#include "consoleCmds.h"
#include "util.h"
#include "safe.h"
#include "drawer.h"
#include "matrix.h"
#include "convex.h"
#include "figure.h"
#include "anim.h"
#include "script.h"

static char *colorGetter(GLfloat *variable);
static int colorSetter(GLfloat *variable, char *value);

#define appendToGetAllExpr(cmd) \
	utilStrRealloc(&getAllScriptExpr, &getAllScriptExprEnd, strlen(cmd)+1); \
	getAllScriptExprEnd=stpcpy(getAllScriptExprEnd, (cmd));
#define addToGetAllExpr(cmd) \
	appendToGetAllExpr(" + '\\n' + "); \
	appendToGetAllExpr(cmd);

#define addCustomGetter(name, getterExpr) \
	consoleCmdsAdd("set "name, 0, 0, getterExpr); \
	consoleCmdsAdd("set "name"?", 0, 0, getterExpr); \
	addToGetAllExpr(getterExpr);
#define addCustom(name, flag, getter, setter) \
	addCustomGetter(name, "'  "name"='+str(gf.get_"getter")"); \
	consoleCmdsAdd("set "name"=", 1, flag, "gf.set_"setter);
#define add(name, flag) \
	addCustom(name, flag, name"()", name"(%)");

#define addBoolGetter(name, getterExpr) \
	consoleCmdsAdd("set "name"?", 0, 0, getterExpr); \
	addToGetAllExpr(getterExpr);
#define addBool(name) \
	addBoolGetter(name, "(gf.get_"name"() and '  "name"' or 'no"name"')"); \
	consoleCmdsAdd("set no"name, 0, 0, "gf.set_"name"(0)"); \
	consoleCmdsAdd("set "name, 0, 0, "gf.set_"name"(1)");

#define addArray(name, flag, minIndex, maxIndex, getter, setter) \
	for (int i=minIndex; i<=maxIndex; i++) { \
		char strName[30], strExpr[30], strExpr2[50]; \
		sprintf(strName, "set "name"%d", i); \
		sprintf(strExpr, "gf.get_"getter, i); \
		consoleCmdsAdd(strName, 0, 0, strExpr); \
		sprintf(strName, "set "name"%d?", i); \
		consoleCmdsAdd(strName, 0, 0, strExpr); \
		sprintf(strExpr2, "'  "name"%d='+str(%s)", i, strExpr); \
		addToGetAllExpr(strExpr2); \
		sprintf(strName, "set "name"%d=", i); \
		sprintf(strExpr, "gf.set_"setter, i); \
		consoleCmdsAdd(strName, 0, flag, strExpr); \
	}

void consoleCmdSetUpdateCmds() {
	consoleCmdsRmBranch("set");
	char *getAllScriptExpr=0, *getAllScriptExprEnd=0;
	appendToGetAllExpr("gf.echo('--- Options ---'");

	add      ("background",    "s");
	addArray ("campos",         0,    3, drawerDim,   "campos(%d)",       "campos(%d,%%)"      );
	addArray ("camposl",        0,    3, drawerDim,   "camposl(%d)",      "camposl(%d,%%)"     );
	addBool  ("convexhull");
	add      ("dimen",          0 );
	add      ("edgesize",       0 );
	add      ("facecolor",     "s");
	add      ("history",        0 );
	add      ("maxfps",         0 );
	addBool  ("pyexpr");
	add      ("selvertcolor",  "s");
	add      ("selvertsize",    0 );
	if (drawerDim>=0) {
		addCustom("spacecolor",  "s",                   "spacecolor(0)",    "spacecolor(0,%)"    );
	}
	addArray ("spacecolor-",   "s",   1, drawerDim,   "spacecolor(-%d)",  "spacecolor(-%d,%%)" );
	addArray ("spacecolor+",   "s",   1, drawerDim,   "spacecolor(%d)",   "spacecolor(%d,%%)"  );
	add      ("speed",          0 );
	addBool  ("stdoutpyexpr");
	add      ("vertsize",       0 );
	appendToGetAllExpr(") or gf.consoleClearAfterCmd()");
	consoleCmdsAdd("set", 0, 0, getAllScriptExpr);
}

#undef appendToGetAllExpr
#undef addToGetAllExpr
#undef addCustomGetter
#undef addCustom
#undef add
#undef addBoolGetter
#undef addBool
#undef addArray


#define throw(msg) {scriptThrowException(msg); return 0;}
#define parseColorAlpha() GLfloat components[4]; colorSetter(components, color)
#define parseColor() parseColorAlpha(); if (components[3]!=1) throw("Alpha channel is not available");
#define checkBounds(min, max, index_meaning) if ((index<min)||(index>max)) throw("Wrong "index_meaning)
#define checkCond(cond) if (!(cond)) throw("Wrong value")

char *consoleCmdGetBackground() {
	return colorGetter(drawerBackColor);
}
void consoleCmdSetBackground(char *color) {
	parseColorAlpha();
	drawerSetBackColor(components);
}

float consoleCmdGetCampos(int index) {
	checkBounds(3, drawerDim, "axis");
	return drawerCamPos[index-1];
}
void consoleCmdSetCampos(int index, float value) {
	checkBounds(3, drawerDim, "axis");
	checkCond((value>drawerVisibleRadius) && (value<=8000000))
	drawerCamPos[index-1]=value;
	drawerSetProjection();
}

float consoleCmdGetCamposl(int index) {
	return log(consoleCmdGetCampos(index))/log(2);
}
void consoleCmdSetCamposl(int index, float value) {
	consoleCmdSetCampos(index, pow(2, value));
}

bool consoleCmdGetConvexhull() {
	return convexHull;
}
void consoleCmdSetConvexhull(bool value) {
	if (convexHull=value)
		convexAttach();
}

int consoleCmdGetDimen() {
	return drawerDim;
}
void consoleCmdSetDimen(bool value)
	throw ("Property is read-only");

float consoleCmdGetEdgesize() {
	return drawerEdgeSize;
}
void consoleCmdSetEdgesize(float value) {
	checkCond(value>=0);
	drawerEdgeSize=value;
	if (drawerVertSize<drawerEdgeSize)
		drawerVertSize=drawerEdgeSize;
	if (drawerSelectedVertSize<drawerEdgeSize)
		drawerSelectedVertSize=drawerEdgeSize;
	drawerSetProjection();
}


char *consoleCmdGetFacecolor() {
	return colorGetter(drawerFaceColor);
}
void consoleCmdSetFacecolor(char *color) {
	colorSetter(drawerFaceColor, color);
}

int consoleCmdGetHistory() {
	return consoleGetHistoryMaxCount();
}
void consoleCmdSetHistory(int value) {
	checkCond(value>=0);
	consoleSetHistoryMaxCount(value);
}

int consoleCmdGetMaxfps() {
	return 1000.0/animFrameDelay;
}
void consoleCmdSetMaxfps(int value) {
	checkCond(value>0);
	animFrameDelay=1000.0/value;
}


bool consoleCmdGetPyexpr() {
	return consoleAllowPythonExpr;
}
void consoleCmdSetPyexpr(bool value) {
	consoleAllowPythonExpr=value;
}


char *consoleCmdGetSelvertcolor() {
	return colorGetter(drawerSelectedVertColor);
}
void consoleCmdSetSelvertcolor(char *color) {
	colorSetter(drawerSelectedVertColor, color);
}

float consoleCmdGetSelvertsize() {
	return drawerSelectedVertSize;
}
void consoleCmdSetSelvertsize(float value) {
	checkCond(value>=0);
	drawerSelectedVertSize=value;
	if (drawerVertSize>drawerSelectedVertSize)
		drawerVertSize=drawerSelectedVertSize;
	if (drawerEdgeSize>drawerSelectedVertSize)
		drawerEdgeSize=drawerSelectedVertSize;
	drawerSetProjection();
}


char *consoleCmdGetSpacecolor(int index) {
	checkBounds(-drawerDim, drawerDim, "axis");
	if (index>0)
		return colorGetter(drawerSpaceColorPositive[index-1]);
	else if (index<0)
		return colorGetter(drawerSpaceColorNegative[-index-1]);
	else
		return colorGetter(drawerSpaceColorCenter);
}
void consoleCmdSetSpacecolor(int index, char *color) {
	checkBounds(-drawerDim, drawerDim, "axis");
	if (index>0)
		colorSetter(drawerSpaceColorPositive[index-1], color);
	else if (index<0)
		colorSetter(drawerSpaceColorNegative[-index-1], color);
	else {
		parseColor();
		matrixCopy(components, drawerSpaceColorCenter, 3);
	}
}

float consoleCmdGetSpeed() {
	return animRotSpeed;
}
void consoleCmdSetSpeed(float value) {
	checkCond(value>=0);
	animRotSpeed=value;
}


bool consoleCmdGetStdoutpyexpr() {
	return consolePythonExprToStdout;
}
void consoleCmdSetStdoutpyexpr(bool value) {
	consolePythonExprToStdout=value;
}


float consoleCmdGetVertsize() {
	return drawerVertSize;
}
void consoleCmdSetVertsize(float value) {
	checkCond(value>=0);
	drawerVertSize=value;
	if (drawerEdgeSize>drawerVertSize)
		drawerEdgeSize=drawerVertSize;
	if (drawerSelectedVertSize<drawerVertSize)
		drawerSelectedVertSize=drawerVertSize;
	drawerSetProjection();
}

#undef throw
#undef parseColorAlpha
#undef parseColor
#undef checkBounds
#undef checkCond


static char *colorGetter(GLfloat *variable) {
	static char string[21];
	unsigned int a, r, g, b;
	if (variable[3]>=0)
		a=variable[3]*255;
	r=variable[0]*255;
	g=variable[1]*255;
	b=variable[2]*255;
	if (variable[3]>=0)
		sprintf(string, "#%.2X%.2X%.2X%.2X", a, r, g, b);
	else
		sprintf(string, "#%.2X%.2X%.2X", r, g, b);
	return string;
}

static int colorSetter(GLfloat *variable, char *value) {
	unsigned int ai=255, ri, gi, bi;
	float a, r, g, b;
	char string[10];
	if ((sscanf(value, "#%2X%2X%2X%2X%1s", &ai, &ri, &gi, &bi, string)==4) || (ai=255, sscanf(value, "#%2X%2X%2X%1s", &ri, &gi, &bi, string)==3)) {
		a=1.0f*ai/255;
		r=1.0f*ri/255;
		g=1.0f*gi/255;
		b=1.0f*bi/255;
	} else if (sscanf(value, "%f #%2X%2X%2X%1s", &a, &ri, &gi, &bi, string)==4) {
		r=1.0f*ri/255;
		g=1.0f*gi/255;
		b=1.0f*bi/255;
	} else if ((sscanf(value, "%f %f %f %f%1s", &a, &r, &g, &b, string)==4) || (a=1, sscanf(value, "%f %f %f%1s", &r, &g, &b, string)==3)) {
	} else if ((sscanf(value, "%f %10s", &a, string)==2) || (a=1, sscanf(value, "%20s", string))) {
		if (strcmp(string, "red")==0)
			{r=1;g=0;b=0;}
		else if (strcmp(string, "green")==0)
			{r=0;g=1;b=0;}
		else if (strcmp(string, "blue")==0)
			{r=0;g=0;b=1;}
		else if (strcmp(string, "yellow")==0)
			{r=1;g=1;b=0;}
		else if (strcmp(string, "cyan")==0)
			{r=0;g=1;b=1;}
		else if (strcmp(string, "purple")==0)
			{r=1;g=0;b=1;}
		else if (strcmp(string, "white")==0)
			{r=1;g=1;b=1;}
		else if (strcmp(string, "black")==0)
			{r=0;g=0;b=0;}
		else if (strcmp(string, "gray")==0)
			{r=0.5;g=0.5;b=0.5;}
		else if ((a==1) && (strcmp(string, "transparent")==0))
			{a=0;r=0;g=0;b=0;}
		else
			return 0;
	} else
		return 0;
	variable[3]=a;
	variable[0]=r;
	variable[1]=g;
	variable[2]=b;
	return 1;
}
