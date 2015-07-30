// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// util contains common useful functions

#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

// Expands path starting with ~/ (home directory) or %/ (app location)
// Returned path is valid till next call (if expanded)
extern char *utilExpandPath(char *ipath);

// Returns path of the application
extern char *utilExecutablePath();


// Reallocs ptr the way that from ptr2 (another pointer in ptr block) is minSize bytes available
// *ptr is pointer to the block
// *ptr2 is another pointer in the block or ptr2==0
// minSize is required size from *ptr2 or *ptr if not set
// both ptr and ptr2 can be changed
extern void utilStrRealloc(char **ptr, char **ptr2, size_t minSize);

// Updates poiner ptr in last realloced block after last calling of utilStrRealloc
extern void utilStrReallocPtrUpdate(char **ptr);


// Returns string length reflecting \b character as in strings module
extern int utilStrLineWidth(char *str);

// List of strings:
	struct utilStrList {
		char *str;
		struct utilStrList *prev;
		struct utilStrList *next;
	};
	extern struct utilStrList *utilStrListAddAfter(struct utilStrList *list);
	extern struct utilStrList *utilStrListRm(struct utilStrList *list);

#endif
