// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// This file is intended to be included only in convexFig.h
// convexFigHash manages global hash table of all existing figures


// hash of figure is a combination of hashes of its vertices (obtained by convexFigHashCalc)
// hash of vertex can be any value (it is expected it to be random)


// Calculates hash of fig except its hash-marked components and combines it with given hash
// Intended use: reset hash-mark, assign 0 to hash, call convexFigHashCalc with all selected figs
// All vertices are hash-marked
extern int convexFigHashCalc(struct convexFig *fig, unsigned int *hash);


// Adds figure to the global hash table
// figure has to have hash calculated
void convexFigHashAdd(struct convexFig *fig);

// Returns a figure from the hash table satisfying:
//   its hash equals given one
//   its dimension is dim
//   its number of vertices equals vertCount
//   all its vertices are hash-marked
extern struct convexFig *convexFigHashFind(unsigned int hash, int dim, int vertCount);

// Removes figures from the global hash table
void convexFigHashRm(struct convexFig *fig);

