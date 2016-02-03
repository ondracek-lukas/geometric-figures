// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "safe.h"

#include <stdio.h>
#include <stdbool.h>

#include "matrix.h"

void *safeMalloc(size_t size) {
	void *mem=malloc(size);
	if (size && !mem)
		safeExitErr("Not enough memory");
	return mem;
};
void *safeCalloc(size_t nmemb, size_t size) {
	void *mem=calloc(nmemb, size);
	if (size && nmemb && !mem)
		safeExitErr("Not enough memory");
	return mem;
};
void *safeRealloc(void *ptr, size_t size) {
	void *mem=realloc(ptr, size);
	if (size && !mem)
		safeExitErr("Not enough memory");
	return mem;
};

void safeExitErr(char *str) {
	fprintf(stderr, "Error occurred: %s\n",str);
	exit(1);
}

bool safeCheckPos(GLdouble *pos, int dim) {
	int i;
	for (i=0; i<dim; i++)
		if ((pos[i]<-safeMaxDist) || (pos[i]>safeMaxDist))
			return 0;
	if (matrixVectorNorm(pos, dim)>safeMaxDist)
		return 0;
	return 1;
}
