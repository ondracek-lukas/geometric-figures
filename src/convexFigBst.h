// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// This file is intended to be included only in convexFig.h
// convexFigBst is binary search tree built on convexFig items sorted by spaces of figures

struct convexSpace;
struct convexFig;
struct convexFigList;

// Internal tree structure (pointer to empty tree is NULL)
struct convexFigBst;

// Adds fig to the given tree
extern int convexFigBstAdd(struct convexFigBst **bst, struct convexFig *fig);

// Removes fig from the tree
extern int convexFigBstRm(struct convexFigBst **bst, struct convexFig *fig);

// Finds all figures generating given space (will be added to list), returns its count
extern int convexFigBstFind(struct convexFigBst *bst, struct convexSpace *space, struct convexFigList **list);

// Adds all figures in bst into list
extern int convexFigBstGetAll(struct convexFigBst *bst, struct convexFigList **list);
