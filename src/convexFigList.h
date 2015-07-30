// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// This file is intended to be included only in convexFig.h
// convexFigList is list of convexFig items

enum convexFigMarkId;
struct convexFig;

// List structure (pointer to empty list is NULL)
struct convexFigList {
	struct convexFig *fig;
	struct convexFigList *next;
};

// Adds fig at the beginning of the list
extern void convexFigListAdd(struct convexFigList **pList, struct convexFig *fig);

// Removes (and returns) the first item of the list
extern struct convexFig *convexFigListRm(struct convexFigList **pList);

// Destroys the list
extern void convexFigListDestroy(struct convexFigList **pList);


// Returns the length of the list
extern int convexFigListLen(struct convexFigList *list);


// Copies all marked figs from src at the beginning of dst, order is preserved
extern void convexFigListCopy(struct convexFigList *src, struct convexFigList **dst, enum convexFigMarkId mark);

// Moves all figs from src at the beginning of dst, order is preserved
extern void convexFigListMove(struct convexFigList **src, struct convexFigList **dst);


// Returns count of occurrences of the fig in the list
extern int convexFigListContains(struct convexFigList *list, struct convexFig *fig);

// Removes all occurrences of the fig from the list, returns its count
extern int convexFigListRmFig(struct convexFigList **pList, struct convexFig *fig);



// Marks all figs in list
extern void convexFigListMarkSet(struct convexFigList *list, enum convexFigMarkId mark);

// Unmarks all figs in list
extern void convexFigListMarkClear(struct convexFigList *list, enum convexFigMarkId mark);
