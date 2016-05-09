// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "drawer.h"

#include <GL/freeglut.h>
#ifndef _GLUfuncptr // not included in mingw32 version
#define _GLUfuncptr void*
#endif

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "safe.h"
#include "matrix.h"
#include "figure.h"
#include "console.h"
#include "consoleCmd.h"
#include "consolePrivDraw.h"
#include "util.h"

GLdouble *drawerCamPos=0;
int drawerDim=0;
GLdouble drawerVertSize=10;
GLdouble drawerEdgeSize=10;
GLdouble drawerSelectedVertSize=15;
const GLdouble drawerVisibleRadius=1.1;
GLfloat drawerFaceColor[4];
GLfloat drawerBackColor[4]={0,0,0,1};
GLfloat (*drawerSpaceColorPositive)[4]=0;
GLfloat (*drawerSpaceColorNegative)[4]=0;
GLfloat drawerSpaceColorCenter[4]={0,0,0,1};
GLfloat drawerSelectedVertColor[4];
int drawerLastDelay=1;
unsigned drawerRedisplayCounter=0;
int drawerWidth=0, drawerHeight=0;
int drawerLastRedisplayTime=0;

static bool redisplayNeeded=false;
static GLfloat stringColor[4];
static GLfloat stringColorRed[4];
static GLfloat stringColorGreen[4];
static GLfloat stringColorBlue[4];
static GLfloat stringColorGray[4];
static GLdouble scale=1;
static GLUquadric *quadric=0;

static void calcSpaceColor(GLfloat *color, GLdouble *pos);
static void drawFigure();
static void drawVert3D(GLdouble *coordinates, GLdouble size, GLfloat *color);
static void drawEdge3D(GLdouble *coords1, GLdouble size1, GLfloat *color1, GLdouble *coords2, GLdouble size2, GLfloat *color2);
static void drawFace3D(int count, GLdouble **coords);
static void drawControls();
static void drawRect(int x1, int y1, int x2, int y2);
static void drawString(char *string, int x, int y);
static struct utilStrList *drawStringMultiline(struct utilStrList *lines, int count, int x, int y);
static void drawBlock();
static void drawStatusLine();

void drawerInit() {
	glClearDepth(1);
	glDepthFunc(GL_LESS);
	quadric=gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);
	gluQuadricNormals(quadric, GLU_SMOOTH);

	drawerResetColors();

	drawerSetDim(-1);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat[]) {0.0f, 0.0f, 1.0f, 0.0f});
}

void drawerResize(int w, int h) {
	drawerWidth=w;
	drawerHeight=h;
	glViewport(0, 0, drawerWidth, drawerHeight);
	drawerSetProjection();
}

void drawerInvokeRedisplay() {
	if (!redisplayNeeded) {
		redisplayNeeded=true;
		glutPostRedisplay();
	}
}
bool drawerWaitingRedisplay() {
	return redisplayNeeded;
}

void drawerDisplay() {
	int time=glutGet(GLUT_ELAPSED_TIME);
	drawerLastDelay=time-drawerLastRedisplayTime;
	drawerLastRedisplayTime=time;
	redisplayNeeded=false;
	drawerRedisplayCounter++;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	drawFigure();
	drawControls();

	glFlush();
	glutSwapBuffers();
}

void drawerSetBackColor(GLfloat *color) {
	glClearColor(
		drawerBackColor[0]=color[0],
		drawerBackColor[1]=color[1],
		drawerBackColor[2]=color[2],
		1);

	if (0.299*color[0]+0.587*color[1]+0.114*color[2]<0.5) {
		matrixCopyF((GLfloat[]) {1.0f, 1.0f, 1.0f}, stringColor, 3);
		if (color[0]<0.75)
			matrixCopyF((GLfloat[]) {1.0f, 0.3f, 0.3f}, stringColorRed, 3);
		else
			matrixCopyF((GLfloat[]) {1.0f, 0.7f, 0.7f}, stringColorRed, 3);
		matrixCopyF((GLfloat[]) {0.05f, 0.9f, 0.05f}, stringColorGreen, 3);
		matrixCopyF((GLfloat[]) {0.5f, 0.5f, 1.0f}, stringColorBlue, 3);
	} else {
		matrixCopyF((GLfloat[]) {0.0f, 0.0f, 0.0f}, stringColor, 3);
		matrixCopyF((GLfloat[]) {0.7f, 0.0f, 0.0f}, stringColorRed, 3);
		matrixCopyF((GLfloat[]) {0.0f, 0.7f, 0.0f}, stringColorGreen, 3);
		matrixCopyF((GLfloat[]) {0.0f, 0.0f, 1.0f}, stringColorBlue, 3);
	}

	matrixZeroF(stringColorGray, 3);
	matrixAddScaledF(stringColorGray, 0.5, stringColor, 3);
	matrixAddScaledF(stringColorGray, 0.5, drawerBackColor, 3);
	drawerInvokeRedisplay();
}

void drawerResetColors() {
	int i;
	matrixCopyF((GLfloat[]) {0.1f, 0.1f, 0.3f, 0.3f}, drawerFaceColor, 4);
	matrixCopyF((GLfloat[]) {1.0f, 1.0f, 1.0f, -1.0f}, drawerSpaceColorCenter, 4);
	matrixCopyF((GLfloat[]) {1.0f, 0.0f, 0.0f, 1.0f}, drawerSelectedVertColor, 4);
	drawerSetBackColor((GLfloat[]) {0.0f, 0.0f, 0.0f, -1.0f});
	for (i=0; i<drawerDim; i++) {
		matrixZeroF(drawerSpaceColorPositive[i], 4);
		matrixZeroF(drawerSpaceColorNegative[i], 4);
	}
	drawerInvokeRedisplay();
}

void drawerSetDim(int dim) {
	int i;
	if (dim>=3) {
		drawerCamPos=((GLdouble *)safeRealloc(drawerCamPos, dim*sizeof(GLdouble)));
		if (drawerDim<3) {
			drawerCamPos[2]=3.5;
			drawerDim=3;
		}
		for (; drawerDim<dim; drawerDim++)
			drawerCamPos[drawerDim]=2;
	} else {
		free(drawerCamPos);
		drawerCamPos=0;
	}
	if (dim>0) {
		drawerSpaceColorPositive=safeRealloc(drawerSpaceColorPositive, dim*sizeof(GLfloat[4]));
		drawerSpaceColorNegative=safeRealloc(drawerSpaceColorNegative, dim*sizeof(GLfloat[4]));
		for (i=0; i<dim; i++) {
			matrixZeroF(drawerSpaceColorPositive[i], 4);
			matrixZeroF(drawerSpaceColorNegative[i], 4);
		}
	}
	drawerDim=dim;
	drawerSetProjection();
	consoleCmdSetUpdateCmds();
	drawerInvokeRedisplay();
}

void drawerFree() {
	if (drawerCamPos>0) {
		free(drawerCamPos);
		gluDeleteQuadric(quadric);
	}
}

void drawerSetProjection() {
	if ((drawerDim<0) || (drawerWidth==0) || (drawerHeight==0))
		return;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (drawerDim<3) {
		if (drawerHeight<drawerWidth)
			glOrtho(
				-drawerVisibleRadius*drawerWidth/drawerHeight, drawerVisibleRadius*drawerWidth/drawerHeight,
				-drawerVisibleRadius, drawerVisibleRadius,
				-drawerVisibleRadius, drawerVisibleRadius);
		else
			glOrtho(
				-drawerVisibleRadius, drawerVisibleRadius,
				-drawerVisibleRadius*drawerHeight/drawerWidth, drawerVisibleRadius*drawerHeight/drawerWidth,
				-drawerVisibleRadius, drawerVisibleRadius);
	} else {
		GLdouble r=sqrt((drawerCamPos[2]-drawerVisibleRadius)/(drawerCamPos[2]+drawerVisibleRadius))*drawerVisibleRadius;
		if (drawerHeight<drawerWidth)
			glFrustum(-r*drawerWidth/drawerHeight, r*drawerWidth/drawerHeight, -r, r, drawerCamPos[2]-drawerVisibleRadius, drawerCamPos[2]+drawerVisibleRadius);
		else
			glFrustum(-r, r, -r*drawerHeight/drawerWidth, r*drawerHeight/drawerWidth, drawerCamPos[2]-drawerVisibleRadius, drawerCamPos[2]+drawerVisibleRadius);
	}
	if (drawerHeight<drawerWidth)
		scale=2*drawerVisibleRadius/drawerHeight;
	else
		scale=2*drawerVisibleRadius/drawerWidth;

	if (drawerVisibleRadius<scale*drawerSelectedVertSize/2) {
		drawerSelectedVertSize=drawerVisibleRadius/scale*2;
		if (drawerVertSize>drawerSelectedVertSize) {
			drawerVertSize=drawerSelectedVertSize;
			if (drawerEdgeSize>drawerVertSize)
				drawerEdgeSize=drawerVertSize;
		}
	}


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (drawerDim>=3)
		glTranslated(0, 0, -drawerCamPos[2]);
	else
		glTranslated(0, 0, -1);
	drawerInvokeRedisplay();
}



// -- private section --


static void calcSpaceColor(GLfloat *color, GLdouble *pos) {
	int i;
	GLfloat alpha=0;
	matrixZeroF(color, 3);
	for (i=0; i<drawerDim; i++) {
		if (pos[i]>0) {
			alpha+=pos[i]*drawerSpaceColorPositive[i][3];
			matrixAddScaledF(color, pos[i]*drawerSpaceColorPositive[i][3], drawerSpaceColorPositive[i], 3);
		} else {
			alpha+=-pos[i]*drawerSpaceColorNegative[i][3];
			matrixAddScaledF(color, -pos[i]*drawerSpaceColorNegative[i][3], drawerSpaceColorNegative[i], 3);
		}
	}
	if (alpha>=1)
		matrixScaleF(color, 1/alpha, 3);
	else
		matrixAddScaledF(color, 1-alpha, drawerSpaceColorCenter, 3);
	color[3]=1;
}

static void drawFigure() {
	int i, j;
	static GLdouble **vertices=0;
	static GLfloat (*vertcolors)[4];
	static GLdouble **facevertc=0;
	int **facevert, facescount;
	static int vertdim=0, vertcount=0;
	GLdouble size1=1, size2=1;

	if (drawerDim<0)
		return;

	if (((drawerDim>=3) && (vertdim!=drawerDim)) || ((drawerDim<=2) && (vertdim!=3)) || (vertcount!=figureData.count[0])) {
		if (vertices!=0) {
			for (i=0; i<vertcount; i++)
				free(vertices[i]);
			free(vertices);
			free(vertcolors);
		}
		vertdim=(drawerDim<=2?3:drawerDim);
		vertcount=figureData.count[0];
		vertices=safeMalloc(vertcount*sizeof(GLdouble *));
		vertcolors=safeMalloc(vertcount*4*sizeof(GLfloat));
		free(facevertc);
		facevertc=safeMalloc(vertcount*sizeof(GLdouble *));
		for (i=0; i<vertcount; i++)
			vertices[i]=safeMalloc(vertdim*sizeof(GLdouble));
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	// vertices
	for (i=0; i<figureData.count[0]; i++) {
		matrixProduct(figureRotMatrix, figureData.vertices[i], vertices[i], drawerDim, drawerDim, 1);
		matrixScale(vertices[i],
			figureScale/drawerVisibleRadius*(drawerVisibleRadius-scale*drawerSelectedVertSize/2),
			drawerDim);
		calcSpaceColor(vertcolors[i], vertices[i]);
		if (drawerDim<=2) {
			vertices[i][2]=0;
			if (drawerDim<=1) {
				vertices[i][1]=0;
				if (drawerDim<=0)
					vertices[i][0]=0;
			}
		} else if (drawerDim>3) {
			matrixPerspective(drawerDim, 3, drawerCamPos, vertices[i]);
			size1=vertices[i][3];
		}
		if (i==consoleCmdVertexSelected) {
			matrixScaleF(vertcolors[i], 1-drawerSelectedVertColor[3], 3);
			matrixAddScaledF(vertcolors[i], drawerSelectedVertColor[3], drawerSelectedVertColor, 3);
			drawVert3D(vertices[i], size1*drawerSelectedVertSize/drawerVertSize, vertcolors[i]);
		} else {
			drawVert3D(vertices[i], size1, vertcolors[i]);
		}
	}

	if (drawerDim>=1) {
		// edges
		for (i=0; i<figureData.count[1]; i++) {
			if (drawerDim>3) {
				size1=vertices[figureData.boundary[1][i][1]][3];
				size2=vertices[figureData.boundary[1][i][2]][3];
			}
			drawEdge3D(
				vertices[figureData.boundary[1][i][1]], size1, vertcolors[figureData.boundary[1][i][1]],
				vertices[figureData.boundary[1][i][2]], size2, vertcolors[figureData.boundary[1][i][2]]);
		}

		if ((drawerDim>=2) && (drawerFaceColor[3]>0)) {
			// faces
			facescount=figureVerticesOfFaces(&facevert);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat[]) {1.0f, 0.0f, 0.0f, 0.5f});
			glDisable(GL_LIGHTING);
			glDepthMask(GL_FALSE);
			glColor4fv(drawerFaceColor);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			for (i=0; i<facescount; i++) {
				for (j=0; j<facevert[i][0]; j++)
					facevertc[j]=vertices[facevert[i][j+1]];
				drawFace3D(facevert[i][0], facevertc);
			}
			glDepthMask(GL_TRUE);
		}
	}
}

static void drawVert3D(GLdouble *coordinates, GLdouble size, GLfloat *color) {
	static GLdouble vertScale=0;
	static GLuint list=0;
	glPushMatrix();
	glTranslated(coordinates[0], coordinates[1], coordinates[2]);
	glScaled(size, size, size);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	if (list==0)
		list=glGenLists(1);
	if (vertScale!=drawerVertSize/2*scale) {
		vertScale=drawerVertSize/2*scale;
		glNewList(list, GL_COMPILE_AND_EXECUTE);
		gluSphere(quadric, vertScale, 20, 20);
		glEndList();
	} else {
		glCallList(list);
	}
	glPopMatrix();
}

static void drawEdge3D(GLdouble *coords1, GLdouble size1, GLfloat *color1, GLdouble *coords2, GLdouble size2, GLfloat *color2) {
	glPushMatrix();
	GLdouble d0=coords2[0]-coords1[0];
	GLdouble d1=coords2[1]-coords1[1];
	GLdouble d2=coords2[2]-coords1[2];
	GLdouble length=sqrt(d0*d0+d1*d1+d2*d2);
	if (length==0) {
		glPopMatrix();
		return;
	}
	glTranslated(coords1[0], coords1[1], coords1[2]);
	if ((d0==0) && (d1==0))
		d0=1;
	glRotated(acos(d2/length)*180*M_1_PI, -d1, d0, 0);

	int i;
	static double sinc[32]= {-1};
	if (sinc[0]==-1)
		for (i=0; i<32; i++)
			sinc[i]=sin(2*M_PI*i/32);
	d1=drawerEdgeSize/2*size1*scale;
	d2=drawerEdgeSize/2*size2*scale;
	glBegin(GL_TRIANGLE_STRIP);
		for (i=0; i<34; i++) {
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (i%2?color2:color1));
			glNormal3d(sinc[i%32], sinc[(i+8)%32], 0);
			glVertex3d((i%2?d2:d1)*sinc[i%32], (i%2?d2:d1)*sinc[(i+8)%32], (i%2)*length);
		}
	glEnd();
	glPopMatrix();
}

static void drawFace3D(int count, GLdouble **coords) {
	int i;
	GLUtesselator *tess=gluNewTess();
	gluTessCallback(tess, GLU_TESS_VERTEX, (_GLUfuncptr)glVertex3dv);
	gluTessCallback(tess, GLU_TESS_BEGIN, (_GLUfuncptr)glBegin);
	gluTessCallback(tess, GLU_TESS_END, (_GLUfuncptr)glEnd);
	gluTessBeginPolygon(tess, 0);
		gluTessBeginContour(tess);
			for (i=0; i<count; i++) {
				gluTessVertex(tess, coords[i], coords[i]);
			}
		gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	gluDeleteTess(tess);
}


static void drawControls() {
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, drawerWidth, 0, drawerHeight, -1, 1);

	drawStatusLine();
	drawBlock();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

static void drawRect(int x1, int y1, int x2, int y2) {
	glBegin(GL_QUADS);
	glVertex2i(x1, y1);
	glVertex2i(x1, y2);
	glVertex2i(x2, y2);
	glVertex2i(x2, y1);
	glEnd();
}

static void drawString(char *str, int x, int y) {
	GLfloat color[4];
	glGetFloatv(GL_CURRENT_COLOR, color);

	glColor4f(drawerBackColor[0], drawerBackColor[1], drawerBackColor[2], 0.8);
	drawRect(x-1, y-5, x+consoleStrWidth(str)*9+1, y+14);
	glColor3fv(color);
	glRasterPos2i(x, y);


	while (*str) {
		switch (*str) {
			case consoleSpecialBack:
				x-=9;
				glRasterPos2i(x, y);
				break;
			case consoleSpecialColorNormal:
				glColor3fv(stringColor);
				glRasterPos2i(x, y);
				break;
			case consoleSpecialColorRed:
				glColor3fv(stringColorRed);
				glRasterPos2i(x, y);
				break;
			case consoleSpecialColorGreen:
				glColor3fv(stringColorGreen);
				glRasterPos2i(x, y);
				break;
			case consoleSpecialColorBlue:
				glColor3fv(stringColorBlue);
				glRasterPos2i(x, y);
				break;
			case consoleSpecialColorGray:
				glColor3fv(stringColorGray);
				glRasterPos2i(x, y);
				break;
				break;
			default:
				x+=9;
				glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *str);
				break;
		}
		str++;
	}
}

static struct utilStrList *drawStringMultiline(struct utilStrList *lines, int count, int x, int y) {
	int i;
	for (i=0; lines && (!count || i<count); i++, lines=lines->next) {
		drawString(lines->str, x, y);
		y-=19;
	}
	return lines;
}

static void drawBlock() {
	int x, y;
	struct utilStrList *list=0;
	if (!consoleBlock)
		return;
	x=(drawerWidth-consoleBlockWidth*9)/2;
	y=(drawerHeight+consoleBlockHeight*19)/2-10;

	if ((x<0) || (y>drawerHeight-10-19)) {
		glColor3fv(stringColorRed);
		utilStrListAddAfter(&list);
		utilStrRealloc(&list->str, 0, 20);
		strcpy(list->str, "Too small window");
		utilStrListAddAfter(&list);
		utilStrRealloc(&list->str, 0, 20);
		sprintf(list->str, "%dx%d needed", consoleBlockWidth*9, consoleBlockHeight*19+2*19);
		utilStrListAddAfter(&list);
		utilStrRealloc(&list->str, 0, 20);
		sprintf(list->str, "current: %dx%d", drawerWidth, drawerHeight);
		list=list->prev;
		list=list->prev;
		x=(drawerWidth>17*9?(drawerWidth-17*9)/2:0);
		y=(drawerHeight>3*19?(drawerHeight+3*19)/2:drawerHeight)-12;
		drawStringMultiline(list, 0, x, y);
		while (list)
			utilStrListRm(&list);
	} else {
		glColor3fv(stringColor);
		drawStringMultiline(consoleBlock, 0, x, y);
	}
}

static void drawStatusLine() {
	int consoleSize=0, count;
	struct utilStrList *lines;

	glColor3fv(stringColor);

	if ((lines=consoleLines)) {
		consoleSize=consoleStrWidth(lines->str);
		count=1;
		while (lines->prev) {
			count++;
			lines=lines->prev;
		}
		drawStringMultiline(lines, 0, 1, 19*count-14);
	}

	glColor3fv(stringColorGreen);
	if (9*(strlen(consoleStatus)+consoleSize+3)<=drawerWidth)
		drawString(consoleStatus, drawerWidth-9*strlen(consoleStatus)-10, 5);
	else if (!consoleLines)
		drawString(consoleStatus, 10, 5);

}
