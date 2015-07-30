// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "drawer.h"

#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "safe.h"
#include "matrix.h"
#include "figure.h"
#include "console.h"
#include "consoleCmd.h"
#include "consolePrivDraw.h"
#include "util.h"

GLfloat *drawerCamPos=0;
int drawerDim=0;
GLfloat drawerVertSize=20;
GLfloat drawerEdgeSize=20;
GLfloat drawerSelectedVertSize=30;
const GLfloat drawerVisibleRadius=1.1;
GLfloat drawerFaceColor[4];
GLfloat drawerBackColor[4];
GLfloat (*drawerSpaceColorPositive)[4]=0;
GLfloat (*drawerSpaceColorNegative)[4]=0;
GLfloat drawerSpaceColorCenter[4];
GLfloat drawerSelectedVertColor[4];

static GLfloat stringColor[4];
static GLfloat stringErrColor[4];
static GLfloat stringStatusColor[4];
static GLfloat scale=1;
static GLUquadric *quadric=0;

int width=0, height=0;

static void calcSpaceColor(GLfloat *color, GLfloat *pos);
static void drawFigure();
static void drawVert3D(GLfloat *coordinates, GLfloat size, GLfloat *color);
static void drawEdge3D(GLfloat *coords1, GLfloat size1, GLfloat *color1, GLfloat *coords2, GLfloat size2, GLfloat *color2);
static void drawFace3D(int count, GLfloat **coords);
static void drawControls();
static void drawRect(int x1, int y1, int x2, int y2);
static void drawString(char *string, int x, int y, GLfloat *color);
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
	width=w;
	height=h;
	glViewport(0, 0, width, height);
	drawerSetProjection();
}

void drawerDisplay() {
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

	if (color[0]+color[1]+color[2]<1.5)
		matrixCopy((GLfloat[]) {1.0f, 1.0f, 1.0f}, stringColor, 3);
	else
		matrixCopy((GLfloat[]) {0.0f, 0.0f, 0.0f}, stringColor, 3);

	if (color[0]+color[1]+color[2]<1.0)
		matrixCopy((GLfloat[]) {1.0f, 0.0f, 0.0f}, stringErrColor, 3);
	else
		matrixCopy((GLfloat[]) {0.5f, 0.0f, 0.0f}, stringErrColor, 3);

	if (color[0]+color[1]+color[2]<1.0)
		matrixCopy((GLfloat[]) {0.0f, 1.0f, 0.0f}, stringStatusColor, 3);
	else
		matrixCopy((GLfloat[]) {0.0f, 0.5f, 0.0f}, stringStatusColor, 3);
}

void drawerResetColors() {
	int i;
	matrixCopy((GLfloat[]) {0.1f, 0.1f, 0.3f, 0.3f}, drawerFaceColor, 4);
	matrixCopy((GLfloat[]) {1.0f, 1.0f, 1.0f, -1.0f}, drawerSpaceColorCenter, 4);
	matrixCopy((GLfloat[]) {1.0f, 0.0f, 0.0f, 1.0f}, drawerSelectedVertColor, 4);
	drawerSetBackColor((GLfloat[]) {0.0f, 0.0f, 0.0f, -1.0f});
	for (i=0; i<drawerDim; i++) {
		matrixZero(drawerSpaceColorPositive[i], 4);
		matrixZero(drawerSpaceColorNegative[i], 4);
	}
}

void drawerSetDim(int dim) {
	int i;
	if (dim>=3) {
		drawerCamPos=((GLfloat *)safeRealloc(drawerCamPos, dim*sizeof(int)));
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
			matrixZero(drawerSpaceColorPositive[i], 4);
			matrixZero(drawerSpaceColorNegative[i], 4);
		}
	}
	drawerDim=dim;
	drawerSetProjection();
}

void drawerFree() {
	if (drawerCamPos>0) {
		free(drawerCamPos);
		gluDeleteQuadric(quadric);
	}
}

void drawerSetProjection() {
	if ((drawerDim<0) || (width==0) || (height==0))
		return;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (drawerDim<3) {
		if (height<width)
			glOrtho(
				-drawerVisibleRadius*width/height, drawerVisibleRadius*width/height,
				-drawerVisibleRadius, drawerVisibleRadius,
				-drawerVisibleRadius, drawerVisibleRadius);
		else
			glOrtho(
				-drawerVisibleRadius, drawerVisibleRadius,
				-drawerVisibleRadius*height/width, drawerVisibleRadius*height/width,
				-drawerVisibleRadius, drawerVisibleRadius);
	} else {
		GLfloat r=sqrt((drawerCamPos[2]-drawerVisibleRadius)/(drawerCamPos[2]+drawerVisibleRadius))*drawerVisibleRadius;
		if (height<width)
			glFrustum(-r*width/height, r*width/height, -r, r, drawerCamPos[2]-drawerVisibleRadius, drawerCamPos[2]+drawerVisibleRadius);
		else
			glFrustum(-r, r, -r*height/width, r*height/width, drawerCamPos[2]-drawerVisibleRadius, drawerCamPos[2]+drawerVisibleRadius);
	}
	if (height<width)
		scale=drawerVisibleRadius/height;
	else
		scale=drawerVisibleRadius/width;

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
		glTranslatef(0, 0, -drawerCamPos[2]);
	else
		glTranslatef(0, 0, -1);
}



// -- private section --


static void calcSpaceColor(GLfloat *color, GLfloat *pos) {
	int i;
	GLfloat alpha=0;
	matrixZero(color, 3);
	for (i=0; i<drawerDim; i++) {
		if (pos[i]>0) {
			alpha+=pos[i]*drawerSpaceColorPositive[i][3];
			matrixAddScaled(color, pos[i]*drawerSpaceColorPositive[i][3], drawerSpaceColorPositive[i], 3);
		} else {
			alpha+=-pos[i]*drawerSpaceColorNegative[i][3];
			matrixAddScaled(color, -pos[i]*drawerSpaceColorNegative[i][3], drawerSpaceColorNegative[i], 3);
		}
	}
	if (alpha>=1)
		matrixScale(color, 1/alpha, 3);
	else
		matrixAddScaled(color, 1-alpha, drawerSpaceColorCenter, 3);
	color[3]=1;
}

static void drawFigure() {
	int i, j;
	static GLfloat **vertices=0;
	static GLfloat (*vertcolors)[4];
	static GLfloat **facevertc=0;
	int **facevert, facescount;
	static int vertdim=0, vertcount=0;
	GLfloat size1=1, size2=1;

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
		vertices=safeMalloc(vertcount*sizeof(GLfloat *));
		vertcolors=safeMalloc(vertcount*4*sizeof(GLfloat));
		free(facevertc);
		facevertc=safeMalloc(vertcount*sizeof(GLfloat *));
		for (i=0; i<vertcount; i++)
			vertices[i]=safeMalloc(vertdim*sizeof(GLfloat));
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
			matrixScale(vertcolors[i], 1-drawerSelectedVertColor[3], 3);
			matrixAddScaled(vertcolors[i], drawerSelectedVertColor[3], drawerSelectedVertColor, 3);
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

static void drawVert3D(GLfloat *coordinates, GLfloat size, GLfloat *color) {
	static GLfloat vertScale=0;
	static GLuint list=0;
	glPushMatrix();
	glTranslatef(coordinates[0], coordinates[1], coordinates[2]);
	glScalef(size, size, size);
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

static void drawEdge3D(GLfloat *coords1, GLfloat size1, GLfloat *color1, GLfloat *coords2, GLfloat size2, GLfloat *color2) {
	glPushMatrix();
	GLfloat d0=coords2[0]-coords1[0];
	GLfloat d1=coords2[1]-coords1[1];
	GLfloat d2=coords2[2]-coords1[2];
	GLfloat length=sqrt(d0*d0+d1*d1+d2*d2);
	if (length==0) {
		glPopMatrix();
		return;
	}
	glTranslatef(coords1[0], coords1[1], coords1[2]);
	if ((d0==0) && (d1==0))
		d0=1;
	glRotatef(acos(d2/length)*180*M_1_PI, -d1, d0, 0);

	int i;
	static float sinc[32]= {-1};
	if (sinc[0]==-1)
		for (i=0; i<32; i++)
			sinc[i]=sin(2*M_PI*i/32);
	d1=drawerEdgeSize/2*size1*scale;
	d2=drawerEdgeSize/2*size2*scale;
	glBegin(GL_TRIANGLE_STRIP);
		for (i=0; i<34; i++) {
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (i%2?color2:color1));
			glNormal3f(sinc[i%32], sinc[(i+8)%32], 0);
			glVertex3f((i%2?d2:d1)*sinc[i%32], (i%2?d2:d1)*sinc[(i+8)%32], (i%2)*length);
		}
	glEnd();
	glPopMatrix();
}

static void drawFace3D(int count, GLfloat **coords) {
	int i;
	GLUtesselator *tess=gluNewTess();
	GLdouble (*data)[3]=safeMalloc(count*sizeof(GLdouble[3]));
	gluTessCallback(tess, GLU_TESS_VERTEX, (_GLUfuncptr)glVertex3fv);
	gluTessCallback(tess, GLU_TESS_BEGIN, (_GLUfuncptr)glBegin);
	gluTessCallback(tess, GLU_TESS_END, (_GLUfuncptr)glEnd);
	gluTessBeginPolygon(tess, 0);
		gluTessBeginContour(tess);
			for (i=0; i<count; i++) {
				data[i][0]=coords[i][0];
				data[i][1]=coords[i][1];
				data[i][2]=coords[i][2];
				gluTessVertex(tess, data[i], coords[i]);
			}
		gluTessEndContour(tess);
	gluTessEndPolygon(tess);
	gluDeleteTess(tess);
	free(data);
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
	glOrtho(0, width, 0, height, -1, 1);

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

static void drawString(char *string, int x, int y, GLfloat *color) {
	glColor4f(drawerBackColor[0], drawerBackColor[1], drawerBackColor[2], 0.8);
	drawRect(x-1, y-5, x+utilStrLineWidth(string)*9+1, y+14);
	glColor3fv(color);
	glRasterPos2i(x, y);

	while (*string) {
		if (*string=='\b') {
			x-=9;
			glRasterPos2i(x, y);
		} else {
			x+=9;
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *string);
		}
		string++;
	}
}

static struct utilStrList *drawStringMultiline(struct utilStrList *lines, int count, int x, int y) {
	int i;
	for (i=0; lines && (!count || i<count); i++, lines=lines->next) {
		drawString(lines->str, x, y, stringColor);
		y-=19;
	}
	return lines;
}

static void drawBlock() {
	int x, y;
	struct utilStrList *list=0;
	if (!consoleBlock)
		return;
	x=(width-consoleBlockWidth*9)/2;
	y=(height+consoleBlockHeight*19)/2-10;
	
	if ((x<0) || (y>height-10-19)) {
		list=utilStrListAddAfter(list);
		utilStrRealloc(&list->str, 0, 20);
		strcpy(list->str, "Too small window");
		list=utilStrListAddAfter(list);
		utilStrRealloc(&list->str, 0, 20);
		sprintf(list->str, "%dx%d needed", consoleBlockWidth*9, consoleBlockHeight*19+2*19);
		list=utilStrListAddAfter(list);
		utilStrRealloc(&list->str, 0, 20);
		sprintf(list->str, "current: %dx%d", width, height);
		list=list->prev;
		list=list->prev;
		x=(width>17*9?(width-17*9)/2:0);
		y=(height>3*19?(height+3*19)/2:height)-12;
		drawStringMultiline(list, 0, x, y);
		while (list)
			list=utilStrListRm(list);
	} else
		drawStringMultiline(consoleBlock, 0, x, y);
}

static void drawStatusLine() {
	int consoleSize=0, count;
	struct utilStrList *lines;
	
	if (consolePrintMode==consolePrintOneLineErr)
		glColor3f(1.0f, 0.0f, 0.0f);
	else
		glColor3f(1.0f, 1.0f, 1.0f);

	switch(consolePrintMode) {
		case consolePrintOneLine:
		case consolePrintOneLineErr:
			consoleSize=strlen(consoleOut)+consoleInLength;
			if (consoleOut[0]!='\0') {
				consoleSize++;
				drawString(consoleOut, 1, 5,
					consolePrintMode==consolePrintOneLine?stringColor:stringErrColor);
			} else if (consoleIsOpen()) {
				drawString(consoleIn, 1+strlen(consoleOut)*9, 5, stringColor);
				drawString("\2", 1+(strlen(consoleOut)+consoleInLength)*9+1, 5, stringColor);
			}
			break;
		case consolePrintMultiline:
			if (lines=consolePrintMultilineList) {
				consoleSize=strlen(lines->str);
				count=1;
				while (lines->prev) {
					count++;
					lines=lines->prev;
				}
				drawStringMultiline(lines, 0, 1, 19*count-10);
			}
			break;
	}

	glColor3f(0.0f, 1.0f, 0.0f);
	if (9*(strlen(consoleStatus)+consoleSize+3)<=width)
		drawString(consoleStatus, width-9*strlen(consoleStatus)-10, 5, stringStatusColor);
	else if ((consoleInLength==0) && (strlen(consoleOut)==0))
		drawString(consoleStatus, 10, 5, stringStatusColor);
	
}
