// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmd.h"

#include <limits.h>
#include <GL/freeglut.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "console.h"
#include "drawer.h"
#include "matrix.h"
#include "convex.h"
#include "figure.h"
#include "anim.h"

static char *colorGetter(GLfloat *variable);
static int colorSetter(GLfloat *variable, char *value);

#include "consoleCmdSetMacrosDef.c.tmp"

	/* macros description:
	variable?(NAME,    // ? is (I-integer|F-float|B-bool|C-color), non-indexed version
		GETTER,
		SETTER_COND,       //  not in bool version
		SETTER
	)
	variables?(NAME, INDEX_MIN, INDEX_MAX, INDEX_MEANING,       // indexed version
		GETTER,
		SETTER_COND,       //  not in bool version
		SETTER
	)
	*/
	/* macros code:
	 * line prefix: (i-indexed|n-non-indexed|*-both)(I-integer|-)(F-float|-)(B-bool|-)(C-color|-):
	 * to see all versions separately run consoleCmdSetMacros.sh without parameters
[ MACROS CODE:
n--B-:	if (printAll || (!skipAll && ((b=(strcmp(name, NAME)==0)) || (strcmp(name, "no" NAME)==0)) && (index<0))) {
i--B-:	if (printAll || (!skipAll && ((b=(strcmp(name, NAME)==0)) || (strcmp(name, "no" NAME)==0)))) {
nIF-C:	if (printAll || (!skipAll && (strcmp(name, NAME)==0) && (index<0))) {
iIF-C:	if (printAll || (!skipAll && (strcmp(name, NAME)==0))) {
iIFBC:		if (!printAll && ((index>INDEX_MAX) || (index<INDEX_MIN)))
iIFBC:			consolePrintErr("Wrong " INDEX_MEANING);
iIFBC:		else
*--B-:		if (value!=0)
*--B-:			consolePrintErr("Cannot assign to bool");
*--B-:		else if (printAll || (deletedChar=='?')) {
*IF-C:		if (value==0) {
iIFBC:			for (printAll && (index=INDEX_MIN); index<=INDEX_MAX; index++) {
*IFBC:				GETTER
iI---:				sprintf(answer, "  " NAME "%d=%d", index, i);
i-F--:				sprintf(answer, "  " NAME "%d=%f", index, f);
i---C:				sprintf(answer, "  " NAME "%d=%s", index, colorGetter(c));
nI---:				sprintf(answer, "  " NAME "=%d", i);
n-F--:				sprintf(answer, "  " NAME "=%f", f);
n---C:				sprintf(answer, "  " NAME "=%s", colorGetter(c));
*--B-:				if (b)
i--B-:					sprintf(answer, "  " NAME "%d", index);
n--B-:					sprintf(answer, "  " NAME);
*--B-:				else
i--B-:					sprintf(answer, "no" NAME "%d", index);
n--B-:					sprintf(answer, "no" NAME);
*IFBC:				consolePrint(answer);
iIFBC:				if (!printAll)
iIFBC:					break;
iIFBC:			}
*IFBC:		} else {
*-F--:			f=strtof(value, &text);
*I---:			i=strtol(value, &text, 10);
*---C:			if (colorSetter(c, value) && SETTER_COND) {
*IF--:			if ((*text=='\0') && SETTER_COND) {
*IFBC:				SETTER
*IF-C:			} else
*IF-C:				consolePrintErr("Wrong value");
*IFBC:		}
*IFBC:		skipAll=!printAll;
*IFBC:	}
] */

void consoleCmdSet(char *text) {
	int printAll, skipAll=0;
	char *name=text;
	char *value=0;
	int index=INT_MIN;
	GLfloat f;
	int i, b;
	GLfloat c[4];
	char answer[80];
	char deletedChar='\0', *deleted=0;
	if (printAll=(*text=='\0'))
		consolePrintMultilineBegin();
	else {
		while ((*text!='\0') && (*text!='=') && (*text!='?') && ((*text<'0') || (*text>'9')))
			text++;
		if ((*text>='0') && (*text<='9')) {
			index=atoi(text);
			deleted=text; deletedChar=*text;
			*(text++)='\0';
			while ((*text>='0') && (*text<='9'))
				text++;
		} else
			index=-1;
		if (*text=='=')
			value=text+1;
		else if ((*text=='?') && (text[1]=='\0'))
			value=0;
		else if (*text!='\0')
			name="";
		if (!deleted) {
			deleted=text; deletedChar=*text;
			*text='\0';
		}
	}

	variableC("background",
		matrixCopy(drawerBackColor, c, 4);,
		(c[3]==1),
		drawerSetBackColor(c);
	)

	variablesF("campos", 3, drawerDim, "axis",
		f=drawerCamPos[index-1];,
		(f>drawerVisibleRadius) && (f<=8000000),
		drawerCamPos[index-1]=f;
		drawerSetProjection();
	)

	variablesF("camposl", 3, drawerDim, "axis",
		f=log(drawerCamPos[index-1])/log(2);,
		(f=pow(2, f), (f>drawerVisibleRadius) && (f<=8000000)),
		drawerCamPos[index-1]=f;
		drawerSetProjection();
	)

	variableB("convexhull",
		b=convexHull;,
		if (convexHull=b)
			convexAttach();
	)

	variableI("dimen",
		i=figureData.dim;,
		(i>=0),
		consolePrintErr("Property is read-only");
	)

	variableF("edgesize",
		f=drawerEdgeSize;,
		(f>=0),
		drawerEdgeSize=f;
		if (drawerVertSize<drawerEdgeSize)
			drawerVertSize=drawerEdgeSize;
		if (drawerSelectedVertSize<drawerEdgeSize)
			drawerSelectedVertSize=drawerEdgeSize;
		drawerSetProjection();
	)

	variableC("facecolor",
		matrixCopy(drawerFaceColor, c, 4);,
		(1),
		matrixCopy(c, drawerFaceColor, 4);
	)

	variableI("history",
		i=consoleGetHistoryMaxCount();,
		(i>=0),
		consoleSetHistoryMaxCount(i);
	)

  variableI("maxfps",
		i=1000.0/animFrameDelay;,
		(i>0),
		animFrameDelay=1000.0/i;
	)

	variableC("selvertcolor",
		matrixCopy(drawerSelectedVertColor, c, 4);,
		(1),
		matrixCopy(c, drawerSelectedVertColor, 4);
	)

	variableF("selvertsize",
		f=drawerSelectedVertSize;,
		(f>=0),
		drawerSelectedVertSize=f;
		if (drawerVertSize>drawerSelectedVertSize)
			drawerVertSize=drawerSelectedVertSize;
		if (drawerEdgeSize>drawerSelectedVertSize)
			drawerEdgeSize=drawerSelectedVertSize;
		drawerSetProjection();
	)

	variableC("spacecolor",
		matrixCopy(drawerSpaceColorCenter, c, 4);,
		(c[3]==1),
		matrixCopy(c, drawerSpaceColorCenter, 3);
	)
	variablesC("spacecolor-", 1, drawerDim, "axis",
		matrixCopy(drawerSpaceColorNegative[index-1], c, 4);,
		(1),
		matrixCopy(c, drawerSpaceColorNegative[index-1], 4);
	)
	variablesC("spacecolor+", 1, drawerDim, "axis",
		matrixCopy(drawerSpaceColorPositive[index-1], c, 4);,
		(1),
		matrixCopy(c, drawerSpaceColorPositive[index-1], 4);
	)

	variableF("speed",
		f=animRotSpeed;,
		(f>=0),
		animRotSpeed=f;
	)

	variableF("vertsize",
		f=drawerVertSize;,
		(f>=0),
		drawerVertSize=f;
		if (drawerEdgeSize>drawerVertSize)
			drawerEdgeSize=drawerVertSize;
		if (drawerSelectedVertSize<drawerVertSize)
			drawerSelectedVertSize=drawerVertSize;
		drawerSetProjection();
	)

	if (!skipAll && !printAll)
		consolePrintErr("Wrong variable name");

	if (deleted)
		*deleted=deletedChar;
}

#include "consoleCmdSetMacrosUndef.c.tmp"

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
