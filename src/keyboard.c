// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "keyboard.h"
#include <string.h>
#include <ctype.h>

#include "anim.h"
#include "safe.h"
#include "convex.h"
#include "console.h"
#include "drawer.h"

struct mappedItem {
	int code;
	char *expr;
	struct animRotation *rot;
	bool pressed;
	struct mappedItem *next; // list of pressed
	struct mappedItem *prev;
};

static struct mappedItem *mapped=0;
static int mappedCount=0;
static int mappedLength=0;
static int pressedCnt=0; // count
static struct mappedItem mappedPressed; // head of the list

static int mappedBS(int code, int first, int last);
static struct mappedItem *createMappedItem(int code);
static struct mappedItem *getMappedItem(int code);
static void userKey(int c, int m, bool pressed);
static void userKeyReleaseAll();

static int codeFromCM(int c, int m);

int mappedBS(int code, int first, int last) {
	int i;
	if (first>=last)
		return first;
	i=(first+last)/2;
	if (mapped[i].code==code)
		return i;
	if (mapped[i].code<code)
		return mappedBS(code, i+1, last);
	else
		return mappedBS(code, first, last-1);
}

void keyboardInit() {
	glutIgnoreKeyRepeat(1);
	mappedLength=1;
	mapped=safeMalloc(sizeof(struct mappedItem));
	mappedPressed.code=0;
	mappedPressed.prev=&mappedPressed;
	mappedPressed.next=&mappedPressed;
}

static struct mappedItem *createMappedItem(int code) {
	userKeyReleaseAll();
	int i=mappedBS(code, 0, mappedCount);
	int j;
	if ((i>=mappedCount) || (mapped[i].code!=code)) {
		if (mappedLength<=mappedCount)
			mapped=safeRealloc(mapped, (mappedLength*=2)*sizeof(struct mappedItem));
		j=mappedCount++;
		for (; j>i; j--)
			mapped[j]=mapped[j-1];
		mapped[i].code=code;
		mapped[i].pressed=0;
		mapped[i].rot=0;
		mapped[i].expr=0;
	}
	return mapped+i;
}

static struct mappedItem *getMappedItem(int code) {
	int i=mappedBS(code, 0, mappedCount-1);
	if ((i>=mappedCount) || (mapped[i].code!=code))
		return 0;
	else
		return mapped+i;
}

void keyboardMap(int code, char *expr) {
	struct mappedItem *item = createMappedItem(code);
	free(item->expr);
	if (expr==0)
		item->expr=0;
	else {
		item->expr=safeMalloc(sizeof(char)*(strlen(expr)+1));
		strcpy(item->expr, expr);
	}
}

struct animRotation *keyboardMapRot(int code, struct animRotation *rot) {
	struct animRotation *rot2;
	userKeyReleaseAll();
	struct mappedItem *item=createMappedItem(code);
	rot2=item->rot;
	item->rot=rot;
	return rot2;
}

struct animRotation *keyboardGetMappedRot(int code) {
	struct mappedItem *item=getMappedItem(code);
	if (!item)
		return 0;
	return item->rot;
}


void userKey(int c, int m, bool pressed) {
	int code=codeFromCM(c, m);
	if (!code)
		return;

	struct mappedItem *item=getMappedItem(code);
	if (item!=0) {
		if (pressed && !(item->pressed)) {
			item->next=&mappedPressed;
			item->prev=mappedPressed.prev;
			mappedPressed.prev->next=item;
			mappedPressed.prev=item;
			if (item->rot)
				animStartRot(item->rot);
		} else if (!pressed && (item->pressed)) {
			if (item->rot)
				animStopRot(item->rot);
			item->prev->next=item->next;
			item->next->prev=item->prev;
		}
		item->pressed=pressed;
	}
	pressedCnt+=(pressed?1:-1);

	if (pressedCnt<=0) {
		// release everything (fix different pressed and released keys)
		userKeyReleaseAll();
	}

	if (pressed && item && item->expr)
		consoleEvalExpr(item->expr);


}

void userKeyReleaseAll() {
	struct mappedItem *item=mappedPressed.next;
	while (item->code) {
		if (item->rot)
			animStopRot(item->rot);
		item->pressed=0;
		item=item->next;
	}
	item->next=item;
	item->prev=item;
	pressedCnt=0;
	item=0;
}


void keyboardPress(int c) {
	// c is +ASCII or -GLUT_KEY
	keyboardPressMod(c, glutGetModifiers());
}

void keyboardPressMod(int c, int mod) {
	// c is +ASCII or -GLUT_KEY
	consoleClearBlock();
	if (convexInteract) {
		userKeyReleaseAll();
		convexInteractKeyPress(c);
	} else if (animSleepActive) {
		userKeyReleaseAll();
		animSleepInterrupt(c, mod);
		return;
	} else if (consoleIsOpen()) {
		userKeyReleaseAll();
		switch(c) {
			case 13: // enter
				consoleEnter();
				break;
			case 27: // escape
				consoleClear();
				break;
			case 8: // backspace
				consoleBackspace();
				break;
			case -GLUT_KEY_UP:
				consoleUp();
				break;
			case -GLUT_KEY_DOWN:
				consoleDown();
				break;
			case -GLUT_KEY_LEFT:
				consoleLeft();
				break;
			case -GLUT_KEY_RIGHT:
				consoleRight();
				break;
			default:
				if ((c>=32) && (c<127)) // printable chars + space
					consoleKeyPress(c);
				break;
		}
	} else
		switch(c) {
			case ':':
				glutIgnoreKeyRepeat(0);
				consoleKeyPress(c);
				break;
			default:
				glutIgnoreKeyRepeat(1);
				consoleClear();
				userKey(c, mod, 1);
				break;
		}
	drawerInvokeRedisplay();
}

void keyboardRelease(int c) {
	if (!convexInteract && !consoleIsOpen()) {
		userKey(c, glutGetModifiers(), 0);
		drawerInvokeRedisplay();
	} else
		userKeyReleaseAll();
}


int codeFromCM(int c, int m) {
	if ((m&2) && (c>=1) && (c<=26))  c+='a'-1;   // Ctrl + letter
	else if (c<0)                c=-c+256;   // special keys

	if (((c>='a') && (c<='z')) || (c>=256) || (c==13) || (c==27) || (c==8) || (c==127))
		return c+(m<<9);     // [Ctrl +] [Alt +] [Shift +] (letter|special|Enter|Esc|BS|Delete)
	else
		return c+((m&4)<<9); // [Alt +] other
}

#define tbl(s2, code) if (strcasecmp(s, s2">")==0) return m+code
int keyboardCodeFromString(char *s) {
	int m=0;
	if (*s=='\0')   return 0;
	if (s[1]=='\0') return *s;
	if (s[0]!='<')  return 0;

	s++;
	if (strchr(s, '-')) {
		while (*s!='-') {
			switch(tolower(*s)) {
				case 's':
					m|=1;
					break;
				case 'c':
					m|=2;
					break;
				case 'a':
					m|=4;
					break;
				default:
					return 0;
			}
			s++;
		}
		s++;
		m<<=9;
	}

	if ((s[1]=='>') && (s[2]=='\0'))
		if (((s[0]>='a') && (s[0]<='z')) || !(m&~(4<<9)))
			return m+*s;
		else
			return 0;

	if (!(m&~(4<<9)))
		tbl("space", 32);
	else
		tbl("space", -m);

	tbl("tab", 9);

	if (m&(2<<9)) m+='a'-1; // Ctrl+Enter = Ctrl+M, ...
	tbl("enter", 13);  tbl("esc", 27);  tbl("bs", 8);
	if (m&(2<<9)) m+=1-'a';

	tbl("delete", 127);

	m+=256; // special keys

	tbl("up", GLUT_KEY_UP);           tbl("down", GLUT_KEY_DOWN);
	tbl("left", GLUT_KEY_LEFT);       tbl("right", GLUT_KEY_RIGHT);
	tbl("home", GLUT_KEY_HOME);       tbl("end", GLUT_KEY_END);
	tbl("pageup", GLUT_KEY_PAGE_UP);  tbl("pagedown", GLUT_KEY_PAGE_DOWN);
	tbl("insert", GLUT_KEY_INSERT);

	tbl("f1", GLUT_KEY_F1);    tbl("f2", GLUT_KEY_F2);    tbl("f3", GLUT_KEY_F3);
	tbl("f4", GLUT_KEY_F4);    tbl("f5", GLUT_KEY_F5);    tbl("f6", GLUT_KEY_F6);
	tbl("f7", GLUT_KEY_F7);    tbl("f8", GLUT_KEY_F8);    tbl("f9", GLUT_KEY_F9);
	tbl("f10", GLUT_KEY_F10);  tbl("f11", GLUT_KEY_F11);  tbl("f12", GLUT_KEY_F12);

	return 0;
}

#undef tbl
#undef tbls

