// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// safe treats some errors using wrapper functions and sets limits
// It can exit application in case of its failure

#ifndef SAFE_H
#define SAFE_H

#include <GL/freeglut.h>
#include <stdbool.h>

// Maximum allowed dimension
#define safeMaxDim 100

// Maximum allowed absolute value of coordinate
#define safeMaxDist 1000.0

// *alloc wrappers, memory is allways allocated (otherwise exits application)
extern void *safeMalloc(size_t size);
extern void *safeCalloc(size_t nmemb, size_t size);
extern void *safeRealloc(void *ptr, size_t size);

// Exits application with given message
extern void safeExitErr(char *str);

// Determines whether coordinates in space are allowed
extern bool safeCheckPos(GLdouble *pos, int dim);

#endif
