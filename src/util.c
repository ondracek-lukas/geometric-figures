// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <GL/freeglut.h>
#include <math.h>

#include "safe.h"

// -- paths --

char *utilExpandPath(char *ipath) { // returned path is valid till next call (if expanded)
	static char *opath=0;
	static int opathSize=0;
	char *prefix=0;
	int prefixLength=0;
	int length;

#ifdef WIN32
	if ((strncmp(ipath, "~\\",2)==0)||(strncmp(ipath,"~/",2)==0)){
		prefix=getenv("userprofile");
		if (prefix)
			prefixLength=strlen(prefix);
	} else if ((strncmp(ipath, "%\\",2)==0)||(strncmp(ipath,"%/",2)==0)){
		prefix=utilExecutablePath();
		if (prefix)
			prefixLength=strrchr(prefix, '\\')-prefix;
	}
#else
	if (strncmp(ipath, "~/",2)==0) {
		prefix=getenv("HOME");
		if (prefix)
			prefixLength=strlen(prefix);
	} else if (strncmp(ipath, "%/",2)==0) {
		prefix=utilExecutablePath();
		if (prefix)
			prefixLength=strrchr(prefix, '/')-prefix;
	}
#endif

	if (!prefix)
		return ipath;

	length=prefixLength+strlen(ipath);
	if (opathSize<length) {
		opathSize=length;
		opath=safeRealloc(opath, opathSize*sizeof(char));
	}
	strncpy(opath, prefix, prefixLength);
#ifdef WIN32
	opath[prefixLength]='\\';
#else
	opath[prefixLength]='/';
#endif
	strcpy(opath+prefixLength+1, ipath+2);
	return opath;
}

char *utilExecutablePath() {
	static char *path=0;
	int size=16;
	int length=0;
	
	if (!path) {
		do {
			path=safeRealloc(path, sizeof(char)*(size*=2));
#ifdef WIN32
			length=GetModuleFileName(0, path, size);
#else
			length=readlink("/proc/self/exe", path, size);
			if (length<0)
				length=readlink("/proc/curproc/file", path, size);
#endif
		} while (length>=size);

		if (length>0) {
			path[length]='\0';
		} else {
			free(path);
			path=0;
		}
	}
	return path;
}


// -- string alloc --

static char *strReallocPtrOld=0;
static char *strReallocPtrNew=0;
void utilStrRealloc(char **ptr, char **ptr2, size_t minSize) {
	// *ptr is pointer to the block
	// *ptr2 is another pointer in the block or ptr2==0
	// minSize is required size from *ptr2 or *ptr if not set
	static struct blocks {
		char *ptr;
		size_t size;
		struct blocks *next;
	} *blocks=0;
	struct blocks *block, **pBlock;

	strReallocPtrOld=*ptr;
	if (!*ptr) {
		block=malloc(sizeof(struct blocks));
		block->size=minSize;
		block->ptr=*ptr=safeMalloc(minSize*sizeof(char));
		block->next=blocks;
		blocks=block;
	} else {
		for (pBlock=&blocks; *pBlock; pBlock=&(*pBlock)->next)
			if ((*pBlock)->ptr == *ptr)
				break;
		if (!(block=*pBlock))
			safeExitErr("String manipulation error"); // !!!
		if (ptr2)
			minSize+=*ptr2-*ptr;
		if (minSize==0) {
			free(*ptr);
			*ptr=0;
			*pBlock=block->next;
			free(block);
		} else if (block->size<minSize) {
			block->size=16;
			while (block->size<minSize) {
				block->size*=2;
				if (block->size==0)
					safeExitErr("Too much memory needed");
			}
			block->ptr=realloc(block->ptr, block->size*sizeof(char));
			*ptr=block->ptr;
		}
	}
	strReallocPtrNew=*ptr;
	if (ptr2)
		utilStrReallocPtrUpdate(ptr2);
}

void utilStrReallocPtrUpdate(char **ptr) {
	*ptr=strReallocPtrNew+(*ptr-strReallocPtrOld);
}


// -- string utils --

void utilStrInsertChar(char *str, char c) {
	char *str2=str;
	while (*str2++); str2++;
	while (str2!=str) {
		*str2=*(str2-1);
		str2--;
	}
	*str2=c;
}

void utilStrRmChars(char *str, int cnt) {
	while (*str) {
		*str=str[cnt];
		str++;
	}
}


// -- string list --

void utilStrListAddAfter(struct utilStrList **pAfter) {
	struct utilStrList *new=safeMalloc(sizeof(struct utilStrList));
	new->str=0;
	new->prev=*pAfter;
	if (*pAfter)
		new->next=(*pAfter)->next;
	else
		new->next=0;
	if (*pAfter && (*pAfter)->next)
		(*pAfter)->next->prev=new;
	if (*pAfter)
		(*pAfter)->next=new;
	*pAfter=new;
}

void utilStrListCopyAfter(struct utilStrList **pAfter, struct utilStrList *list) {
	while (list) {
		utilStrListAddAfter(pAfter);
		utilStrRealloc(&(*pAfter)->str, 0, strlen(list->str)+1);
		strcpy((*pAfter)->str, list->str);
		list=list->next;
	}
}

struct utilStrList *utilStrListOfLines(char *str) {
	struct utilStrList *lines=0;
	char *lineEnd;
	char *str2;
	do {
		lineEnd=str;
		utilStrListAddAfter(&lines);
		while (*lineEnd && (*lineEnd!='\n'))
			lineEnd++;
		utilStrRealloc(&lines->str, 0, lineEnd-str+1);
		str2=lines->str;
		while (str!=lineEnd)
			*str2++ = *str++;
		*str2++='\0';
	} while (*str++);
	if (lines)
		while (lines->prev)
			lines=lines->prev;
	return lines;
}

void utilStrListRm(struct utilStrList **pList) {
	struct utilStrList *node=*pList;
	if (node->next)
		node->next->prev=node->prev;
	if (node->prev)
		node->prev->next=node->next;
	if (node->next)
		*pList=node->next;
	else
		*pList=node->prev;
	utilStrRealloc(&node->str, 0, 0);
	free(node);
}
