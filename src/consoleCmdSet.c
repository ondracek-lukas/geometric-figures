// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmd.h"

#include <limits.h>
#include <GL/freeglut.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "console.h"
#include "consoleTransl.h"
#include "util.h"
#include "safe.h"
#include "drawer.h"
#include "matrix.h"
#include "convex.h"
#include "figure.h"
#include "anim.h"
#include "script.h"
#include "hid.h"

static char *colorGetter(GLfloat *variable);
static bool colorSetter(GLfloat *variable, char *value);

#define appendToGetAllExpr(cmd) \
	utilStrRealloc(&getAllScriptExpr, &getAllScriptExprEnd, strlen(cmd)+1); \
	getAllScriptExprEnd=stpcpy(getAllScriptExprEnd, (cmd));
#define addToGetAllExpr(cmd) \
	appendToGetAllExpr(" + '\\n' + "); \
	appendToGetAllExpr(cmd);

#define addCustomGetter(name, getterExpr) \
	consoleTranslAdd("set "name, getterExpr, 0, 0, false); \
	consoleTranslAdd("set "name"?", getterExpr, 0, 0, true); \
	addToGetAllExpr(getterExpr);
#define addCustom(name, flag, getter, setter) \
	addCustomGetter(name, "'  "name"='+str(gf.get_"getter")"); \
	consoleTranslAdd("set "name"=", "gf.set_"setter, 1, flag, false);
#define add(name, flag) \
	addCustom(name, flag, name"()", name"(%)");

#define addBoolGetter(name, getterExpr) \
	consoleTranslAdd("set "name"?", getterExpr, 0, 0, false); \
	addToGetAllExpr(getterExpr);
#define addBool(name) \
	addBoolGetter(name, "(gf.get_"name"() and '  "name"' or 'no"name"')"); \
	consoleTranslAdd("set no"name, "gf.set_"name"(0)", 0, 0, false); \
	consoleTranslAdd("set "name, "gf.set_"name"(1)", 0, 0, false);

#define addArray(name, flag, minIndex, maxIndex, getter, setter) \
	for (int i=minIndex; i<=maxIndex; i++) { \
		char strName[30], strExpr[30], strExpr2[50]; \
		sprintf(strName, "set "name"%d", i); \
		sprintf(strExpr, "gf.get_"getter, i); \
		consoleTranslAdd(strName, strExpr, 0, 0, false); \
		sprintf(strName, "set "name"%d?", i); \
		consoleTranslAdd(strName, strExpr, 0, 0, true); \
		sprintf(strExpr2, "'  "name"%d='+str(%s)", i, strExpr); \
		addToGetAllExpr(strExpr2); \
		sprintf(strName, "set "name"%d=", i); \
		sprintf(strExpr, "gf.set_"setter, i); \
		consoleTranslAdd(strName, strExpr, 1, flag, false); \
	}

void consoleCmdSetUpdateCmds() {
	consoleTranslRmBranch("set");
	char *getAllScriptExpr=0, *getAllScriptExprEnd=0;
	appendToGetAllExpr("gf.echo('--- Options ---'");

	add      ("background",    "c");
	addArray ("campos",         0,    3, drawerDim,   "campos(%d)",       "campos(%d,%%)"      );
	addArray ("camposl",        0,    3, drawerDim,   "camposl(%d)",      "camposl(%d,%%)"     );
	addBool  ("convexhull");
	add      ("dimen",          0 );
	add      ("edgesize",       0 );
	add      ("facecolor",     "C");
	addBool  ("grabmouse");
	add      ("history",        0 );
	add      ("maxfps",         0 );
	add      ("mousesens",      0 );
	addBool  ("pyexpr");
	add      ("selvertcolor",  "C");
	add      ("selvertsize",    0 );
	if (drawerDim>=0) {
		addCustom("spacecolor",  "c",                   "spacecolor(0)",    "spacecolor(0,%)"    );
	}
	addArray ("spacecolor-",   "C",   1, drawerDim,   "spacecolor(-%d)",  "spacecolor(-%d,%%)" );
	addArray ("spacecolor+",   "C",   1, drawerDim,   "spacecolor(%d)",   "spacecolor(%d,%%)"  );
	add      ("speed",          0 );
	addBool  ("stdoutpyexpr");
	add      ("vertsize",       0 );
	appendToGetAllExpr(") or gf.clearAfterCmd()");
	consoleTranslAdd("set", getAllScriptExpr, 0, 0, false);
}

#undef appendToGetAllExpr
#undef addToGetAllExpr
#undef addCustomGetter
#undef addCustom
#undef add
#undef addBoolGetter
#undef addBool
#undef addArray


#define throwS(msg) {scriptThrowException(msg); return;}
#define throwG(msg) {scriptThrowException(msg); return 0;}
#define parseColorAlpha() GLfloat components[4]; if(!colorSetter(components, color)) return
#define parseColor() parseColorAlpha(); if (components[3]!=1) throwS("Alpha channel is not available");
#define checkBoundsG(min, max, index_meaning) if ((index<min)||(index>max)) throwG("Wrong "index_meaning)
#define checkBoundsS(min, max, index_meaning) if ((index<min)||(index>max)) throwS("Wrong "index_meaning)
#define checkCond(cond) if (!(cond)) throwS("Wrong value")

char *consoleCmdGetBackground() {
	return colorGetter(drawerBackColor);
}
void consoleCmdSetBackground(char *color) {
	parseColorAlpha();
	drawerSetBackColor(components);
}

double consoleCmdGetCampos(int index) {
	checkBoundsG(3, drawerDim, "axis");
	return drawerCamPos[index-1];
}
void consoleCmdSetCampos(int index, double value) {
	checkBoundsS(3, drawerDim, "axis");
	checkCond((value>drawerVisibleRadius) && (value<=8000000))
	drawerCamPos[index-1]=value;
	drawerSetProjection();
}

double consoleCmdGetCamposl(int index) {
	return log(consoleCmdGetCampos(index))/log(2);
}
void consoleCmdSetCamposl(int index, double value) {
	consoleCmdSetCampos(index, pow(2, value));
}

bool consoleCmdGetConvexhull() {
	return convexHull;
}
void consoleCmdSetConvexhull(bool value) {
	if ((convexHull=value)) {
		convexAttach(&figureData);
	}
}

int consoleCmdGetDimen() {
	return drawerDim;
}
void consoleCmdSetDimen(bool value)
	throwS ("Property is read-only");

double consoleCmdGetEdgesize() {
	return drawerEdgeSize;
}
void consoleCmdSetEdgesize(double value) {
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
	drawerInvokeRedisplay();
}

bool consoleCmdGetGrabmouse() {
	return hidGrabMouse;
}
void consoleCmdSetGrabmouse(bool value) {
	hidGrabMouse=value;
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

double consoleCmdGetMousesens() {
	return hidMouseSensitivity;
}
void consoleCmdSetMousesens(double value) {
	hidMouseSensitivity=value;
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
	drawerInvokeRedisplay();
}

double consoleCmdGetSelvertsize() {
	return drawerSelectedVertSize;
}
void consoleCmdSetSelvertsize(double value) {
	checkCond(value>=0);
	drawerSelectedVertSize=value;
	if (drawerVertSize>drawerSelectedVertSize)
		drawerVertSize=drawerSelectedVertSize;
	if (drawerEdgeSize>drawerSelectedVertSize)
		drawerEdgeSize=drawerSelectedVertSize;
	drawerSetProjection();
}

char *consoleCmdGetSpacecolor(int index) {
	checkBoundsG(-drawerDim, drawerDim, "axis");
	if (index>0)
		return colorGetter(drawerSpaceColorPositive[index-1]);
	else if (index<0)
		return colorGetter(drawerSpaceColorNegative[-index-1]);
	else
		return colorGetter(drawerSpaceColorCenter);
}
void consoleCmdSetSpacecolor(int index, char *color) {
	checkBoundsS(-drawerDim, drawerDim, "axis");
	if (index>0)
		colorSetter(drawerSpaceColorPositive[index-1], color);
	else if (index<0)
		colorSetter(drawerSpaceColorNegative[-index-1], color);
	else {
		parseColor();
		matrixCopyF(components, drawerSpaceColorCenter, 3);
	}
	drawerInvokeRedisplay();
}

double consoleCmdGetSpeed() {
	return animRotSpeed;
}
void consoleCmdSetSpeed(double value) {
	checkCond(value>=0);
	animRotSpeed=value;
}

bool consoleCmdGetStdoutpyexpr() {
	return consolePythonExprToStdout;
}
void consoleCmdSetStdoutpyexpr(bool value) {
	consolePythonExprToStdout=value;
}

double consoleCmdGetVertsize() {
	return drawerVertSize;
}
void consoleCmdSetVertsize(double value) {
	checkCond(value>=0);
	drawerVertSize=value;
	if (drawerEdgeSize>drawerVertSize)
		drawerEdgeSize=drawerVertSize;
	if (drawerSelectedVertSize<drawerVertSize)
		drawerSelectedVertSize=drawerVertSize;
	drawerSetProjection();
}

#undef throwG
#undef throwS
#undef parseColorAlpha
#undef parseColor
#undef checkBoundsG
#undef checkBoundsS
#undef checkCond


static char *colorGetter(GLfloat *variable) {
	static char string[21];
	unsigned int a, r, g, b;
	if (variable[3]>=0)
		a=variable[3]*255;
	r=variable[0]*255;
	g=variable[1]*255;
	b=variable[2]*255;
	if ((variable[3]>=0)&&(variable[3]<1))
		sprintf(string, "#%.2X%.2X%.2X%.2X", a, r, g, b);
	else
		sprintf(string, "#%.2X%.2X%.2X", r, g, b);
	return string;
}

static bool colorSetter(GLfloat *variable, char *value) {
	unsigned int ai=255, ri, gi, bi;
	char string[10];
	value=consoleTranslColorANormalize(value);
	if (!value)
		return false;
	if ((sscanf(value, "#%2X%2X%2X%2X%1s", &ai, &ri, &gi, &bi, string)==4) || (ai=255, sscanf(value, "#%2X%2X%2X%1s", &ri, &gi, &bi, string)==3)) {
		variable[3]=1.0f*ai/255;
		variable[0]=1.0f*ri/255;
		variable[1]=1.0f*gi/255;
		variable[2]=1.0f*bi/255;
		return true;
	}
	scriptThrowException("Wrong color");
	return false;
}
