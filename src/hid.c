// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "hid.h"
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>

#include "util.h"
#include "anim.h"
#include "safe.h"
#include "convex.h"
#include "console.h"
#include "drawer.h"


// -- codes --

// Keyboard:      00000000 00000ACS P0000000 ___ascii
//                00000000 00000ACS P0000001 _special
// Mouse buttons: 00000000 00000ACS P10000_____button
// Mouse move:    00000000 00000ACS 01XY00____buttons
//
//           ACS: Alt, Control, Shift
//             P: Press (Release otherwise)
//           X/Y: Mouse move in X/Y axis
//       special: GLUT_KEY_...
//        button: (1<<mouse_button_number)

#define FLAG_PRESS          (1<<15)
#define FLAG_MOUSE          (1<<14)
#define FLAG_MOUSE_X        (1<<13)
#define FLAG_MOUSE_Y        (1<<12)
#define FLAGS_MOUSE_BUTTONS 0x3ff

#define SHIFT_MOD_FLAGS     16
#define FLAG_SPECIAL        (1<<8)
#define FLAG_SHIFT          (GLUT_ACTIVE_SHIFT<< SHIFT_MOD_FLAGS)
#define FLAG_ALT            (GLUT_ACTIVE_ALT  << SHIFT_MOD_FLAGS)
#define FLAG_CTRL           (GLUT_ACTIVE_CTRL << SHIFT_MOD_FLAGS)

#define KEY_ENTER 13
#define KEY_ESC   27
#define KEY_BS     8
#define KEY_DEL  127

int hidCodeFromEvent(int keyCode, bool isSpecial, int modifiers, bool pressed) {
	int c=keyCode, m=modifiers;
	if (isSpecial) c|=FLAG_SPECIAL;                  // special keys
	else if ((m&2) && (c>=1) && (c<=26))  c+='a'-1; // Ctrl + letter
	if (pressed)   c|=FLAG_PRESS;

	if (((c>='a') && (c<='z')) || (c & FLAG_SPECIAL) || (c==KEY_ENTER) || (c==KEY_ESC) || (c==KEY_BS) || (c==KEY_DEL))
		return c | (m<<SHIFT_MOD_FLAGS);             // [Ctrl +] [Alt +] [Shift +] (letter|special|Enter|Esc|BS|Delete)
	else
		return c | ((m<<SHIFT_MOD_FLAGS) & FLAG_ALT); // [Alt +] other
}

int hidCodeFromMouseEvent(int button, int modifiers, bool pressed) {
	if ((button<0) || (button>9))
		return 0;
	return FLAG_MOUSE | pressed*FLAG_PRESS | (1<<button) | (modifiers<<SHIFT_MOD_FLAGS);
}

#define tbl(s2, code) if (strcasecmp(s, s2">")==0) return m+code
int hidCodeFromString(char *s) {
	int m=FLAG_PRESS;
	if (*s=='\0')   return 0;
	if (s[1]=='\0') return *s | FLAG_PRESS;
	if (s[0]!='<')  return 0;

	s++;
	if (strchr(s, '-')) {
		while (*s!='-') {
			switch(tolower(*s)) {
				case 's':
					m|=FLAG_SHIFT;
					break;
				case 'c':
					m|=FLAG_CTRL;
					break;
				case 'a':
					m|=FLAG_ALT;
					break;
				case 'r':
					m&=~FLAG_PRESS;
					break;
				default:
					return 0;
			}
			s++;
		}
		s++;
	}

	if ((s[1]=='>') && (s[2]=='\0')) {
		if (((s[0]>='a') && (s[0]<='z')) || !(m & (FLAG_CTRL|FLAG_SHIFT))) {
			return m | *s;
		} else {
			return 0;
		}
	}

	if (!(m & (FLAG_CTRL|FLAG_SHIFT))) {
		tbl("space", 32);
	} else {
		tbl("space", -m);
	}

	tbl("tab", 9);

	if (m & FLAG_CTRL) m+='a'-1; // Ctrl+Enter = Ctrl+M, ...
	tbl("enter", KEY_ENTER);  tbl("esc", KEY_ESC);  tbl("bs", KEY_BS);
	if (m & FLAG_CTRL) m+=1-'a';

	tbl("delete", 127);

	m|=FLAG_SPECIAL; // special keys

	tbl("up", GLUT_KEY_UP);           tbl("down", GLUT_KEY_DOWN);
	tbl("left", GLUT_KEY_LEFT);       tbl("right", GLUT_KEY_RIGHT);
	tbl("home", GLUT_KEY_HOME);       tbl("end", GLUT_KEY_END);
	tbl("pageup", GLUT_KEY_PAGE_UP);  tbl("pagedown", GLUT_KEY_PAGE_DOWN);
	tbl("insert", GLUT_KEY_INSERT);

	tbl("f1", GLUT_KEY_F1);    tbl("f2", GLUT_KEY_F2);    tbl("f3", GLUT_KEY_F3);
	tbl("f4", GLUT_KEY_F4);    tbl("f5", GLUT_KEY_F5);    tbl("f6", GLUT_KEY_F6);
	tbl("f7", GLUT_KEY_F7);    tbl("f8", GLUT_KEY_F8);    tbl("f9", GLUT_KEY_F9);
	tbl("f10", GLUT_KEY_F10);  tbl("f11", GLUT_KEY_F11);  tbl("f12", GLUT_KEY_F12);

	m&=~FLAG_SPECIAL;

	if (strncmp(s, "mouse", 5)==0) {
		s+=5;
		m|=FLAG_MOUSE;
		int axes=0;
		while ((*s>='0') && (*s<='9')) {
			m |= 1<<(*s++-'0');
			axes++;
		}
		if (axes==0)
			return 0;
		if (*s=='x') {
			m|= FLAG_MOUSE_X;
			m&=~FLAG_PRESS;
			s++;
		} else if (*s=='y') {
			m|=FLAG_MOUSE_Y;
			m&=~FLAG_PRESS;
			s++;
		} else if (axes>1) {
			return 0;
		}
		if ((s[0]=='>') && (s[1]=='\0'))
			return m;
		else
			return 0;
	}

	return 0;
}

#undef tbl
#undef tbls


// -- mapping --

struct mappedItem {
	int code;
	char *expr[2]; // release, press
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
static void userKeyReleaseAll();

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

void hidInit() {
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
		mapped[i].expr[0]=0;
		mapped[i].expr[1]=0;
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

void hidMap(int code, char *expr) {
	char **dstExpr;
	bool press = code & FLAG_PRESS;
	code &= ~FLAG_PRESS;
	struct mappedItem *item = createMappedItem(code);
	dstExpr = item->expr + press;
	free(*dstExpr);
	if (expr==0)
		*dstExpr=0;
	else {
		*dstExpr=safeMalloc(sizeof(char)*(strlen(expr)+1));
		strcpy(*dstExpr, expr);
	}
}

struct animRotation *hidMapRot(int code, struct animRotation *rot) {
	struct animRotation *rot2;
	userKeyReleaseAll();
	struct mappedItem *item=createMappedItem(code & ~FLAG_PRESS);
	rot2=item->rot;
	item->rot=rot;
	return rot2;
}

struct animRotation *hidGetMappedRot(int code) {
	struct mappedItem *item=getMappedItem(code);
	if (!item)
		return 0;
	return item->rot;
}


// -- event handling --

static int mouseState=0;
static int mouseX, mouseY;
static struct mappedItem *mouseXItem, *mouseYItem;
static bool mouseGrabbed=false;
static struct waitingEvent {
	int code;
	struct waitingEvent *next;
} *waitingEvents=0, *waitingEventsEnd=0, *waitingEventsFreed=0;
static bool allEventsWaiting=false;

double hidMouseSensitivity=0.5;
bool hidGrabMouse=false;

static void addWaitingEvent(int code) {
	if (!code)
		return;
	allEventsWaiting=true;
	struct waitingEvent *event;
	if (waitingEventsFreed) {
		event=waitingEventsFreed;
		waitingEventsFreed=event->next;
	} else {
		event=malloc(sizeof(struct waitingEvent));
	}
	event->next=0;
	event->code=code;
	if (waitingEventsEnd)
		waitingEventsEnd->next=event;
	else
		waitingEvents=event;
	waitingEventsEnd=event;
}
static void doWaitingEvents(int nothing) {
	allEventsWaiting=false;
	while (waitingEvents) {
		struct waitingEvent *event=waitingEvents;
		waitingEvents=event->next;
		if (!waitingEvents)
			waitingEventsEnd=0;
		event->next=waitingEventsFreed;
		waitingEventsFreed=event;
		hidKeyEvent(event->code);
	}
}
void hidInvokeWaitingEvents() {
	glutTimerFunc(0, doWaitingEvents, 0);
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

static inline void grabMouse() {
	if (hidGrabMouse && !mouseGrabbed) {
		glutSetCursor(GLUT_CURSOR_NONE);
		mouseGrabbed=true;
	}
}
static inline void releaseMouse() {
	if (mouseGrabbed) {
		glutWarpPointer(mouseX, mouseY);
		glutSetCursor(GLUT_CURSOR_INHERIT);
		mouseGrabbed=false;
	}
}
static inline void updateMouseRots(int modifiers) {
	if (mouseState) {
		mouseXItem=getMappedItem((modifiers<<SHIFT_MOD_FLAGS) | FLAG_MOUSE | mouseState | FLAG_MOUSE_X);
		mouseYItem=getMappedItem((modifiers<<SHIFT_MOD_FLAGS) | FLAG_MOUSE | mouseState | FLAG_MOUSE_Y);
	} else {
		mouseXItem=0;
		mouseYItem=0;
	}
}

void hidKeyEvent(int code) {
	if (allEventsWaiting) {
		addWaitingEvent(code);
		return;
	}
	int wholeCode=code;
	bool pressed = code & FLAG_PRESS;
	code &= ~FLAG_PRESS;
	if (!code)
		return;

	if (pressed)
		consoleClearBlock();

	if (code & FLAG_MOUSE) {
		int button = code & FLAGS_MOUSE_BUTTONS;
		if (pressed) {
			if (!mouseState)
				grabMouse();
			mouseState |= button;
		} else {
			mouseState &=~button;
			if (!mouseState)
				releaseMouse();
		}
		updateMouseRots(code & (FLAG_SHIFT|FLAG_CTRL|FLAG_ALT));
	}

	if (convexInteract) {

		userKeyReleaseAll();
		if (pressed) {
			convexInteractKeyPress();
		}

	} else if (animSleepActive) {

		struct mappedItem *item;
		if (pressed || ((item=getMappedItem(code)) && item->expr[pressed])) {
			userKeyReleaseAll();
			addWaitingEvent(wholeCode);
			animSleepInterrupt();
		}

	} else if (consoleIsOpen() && !(code & FLAG_MOUSE)) {

		userKeyReleaseAll();
		if (pressed) {
			switch(code) {
				case KEY_ENTER:
					consoleEnter();
					break;
				case KEY_ESC:
					consoleClear();
					break;
				case KEY_BS:
					consoleBackspace();
					break;
				case KEY_DEL:
					consoleDelete();
					break;
				case '\t':
					consoleTab();
					break;
				case FLAG_SPECIAL|GLUT_KEY_UP:
					consoleUp();
					break;
				case FLAG_SPECIAL|GLUT_KEY_DOWN:
					consoleDown();
					break;
				case FLAG_SPECIAL|GLUT_KEY_LEFT:
					consoleLeft();
					break;
				case FLAG_SPECIAL|GLUT_KEY_RIGHT:
					consoleRight();
					break;
				case FLAG_SPECIAL|GLUT_KEY_HOME:
					consoleHome();
					break;
				case FLAG_SPECIAL|GLUT_KEY_END:
					consoleEnd();
					break;
				default:
					if ((code>=32) && (code<127)) // printable chars + space
						consoleKeyPress(code);
					break;
			}
		}

	} else if (pressed && (code==':')) {

		glutIgnoreKeyRepeat(0);
		consoleKeyPress(code);

	} else { // mapped

		glutIgnoreKeyRepeat(1);
		struct mappedItem *item=getMappedItem(code);
		if (pressed || (item && item->expr[pressed])) {
			consoleClear();
		}
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

		if (item && item->expr[pressed]) {
			consoleEvalExpr(item->expr[pressed]);
		}

	}
}

void evalExprSubstr(char *expr, int param) {
	static char *finalExpr=0;
	utilStrRealloc(&finalExpr, 0, strlen(expr) + 32);
	char *str=finalExpr;
	while (*expr && (*expr!='%'))
		*str++=*expr++;
	if (*expr++=='%') {
		str+=sprintf(str, "%d", param);
		while (*expr)
			*str++=*expr++;
	}
	*str='\0';
	consoleEvalExpr(finalExpr);
}

void hidMouseMoveEvent(int x, int y, int modifiers) {
	static int originX, originY;
	static int warpingX=INT_MIN, warpingY;
	if ((x==warpingX) && (y==warpingY))
		warpingX=INT_MIN;
	else if (!mouseState) {
		mouseX=x;
		mouseY=y;
	} else {
		if (animSleepActive) {
			animSleepInterrupt();
			return;
		}
		int diffX=x-originX;
		int diffY=y-originY;
		if ((diffX==0) && (diffY==0))
			return;
		updateMouseRots(modifiers);
		if (!(diffX && mouseXItem) && !(diffY && mouseYItem))
			return;
		consoleClear();
		if (diffX && mouseXItem) {
			if (mouseXItem->rot)
				animCustomRot(mouseXItem->rot, diffX*hidMouseSensitivity);
			if (mouseXItem->expr[0])
				evalExprSubstr(mouseXItem->expr[0], diffX);
		}

		if (diffY && mouseYItem) {
			if (mouseYItem->rot)
				animCustomRot(mouseYItem->rot, -diffY*hidMouseSensitivity);
			if (mouseYItem->expr[0])
				evalExprSubstr(mouseYItem->expr[0], diffY);
		}

		if (hidGrabMouse) {
			if (!mouseGrabbed) {
				grabMouse();
				mouseX=x;
				mouseY=y;
			}
			warpingX=drawerWidth/2;
			warpingY=drawerHeight/2;
			glutWarpPointer(warpingX, warpingY);
		} else {
			if (mouseGrabbed)
				releaseMouse();
		}
	}
	originX=x;
	originY=y;
}
