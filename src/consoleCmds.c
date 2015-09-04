// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmds.h"

#include <string.h>
#include <stdbool.h>
#include <dirent.h>

#include "util.h"
#include "debug.h"
#include "safe.h"
#include "script.h"

#define isDelim(c) (((c)<'0')||(((c)>'9')&&((c)<'A'))||(((c)>'Z')&&((c)<'a'))||((c)>'z'))

// -- trie manipulation --

enum importance {
	builtinCmd=2,
	userCmd=1,
	builtinAliasCmd=0
};

struct trie {
	char c;
	struct trie *sibling;
	struct trie *child;
	char *scriptExpr;
	char *paramsFlags;
	int params;
	enum importance importance;
};


static inline struct trie **trieGetChildPtr(struct trie *trie, char c) {
	struct trie **pTrie;
	pTrie=&trie->child;
	while (*pTrie && ((*pTrie)->c != c))
		pTrie=&(*pTrie)->sibling;
	return pTrie;
}

static inline struct trie *trieGetChild(struct trie *trie, char c) {
	trie=trie->child;
	while (trie && (trie->c != c))
		trie=trie->sibling;
	return trie;
}

static inline struct trie *trieGetChildCreate(struct trie *trie, char c) {
	struct trie **pTrie=&trie->child;
	while (*pTrie && ((*pTrie)->c != c))
		pTrie=&(*pTrie)->sibling;
	if (!*pTrie) {
		*pTrie=calloc(1, sizeof(struct trie));
		(*pTrie)->c=c;
	}
	return *pTrie;
}

static void trieDestroy(struct trie **pTrie, enum importance imp) {
	for (struct trie **pt=&(*pTrie)->child, *t2=*pt; *pt; (*pt!=t2) || (pt=&(*pt)->sibling), (t2=*pt))
		trieDestroy(pt, imp);

	if (!(*pTrie)->scriptExpr || ((*pTrie)->importance == imp)) {
		if ((*pTrie)->child) {
			(*pTrie)->scriptExpr=0;
		} else {
			struct trie *t=*pTrie;
			(*pTrie)=t->sibling;
			free(t->scriptExpr);
			free(t->paramsFlags);
			free(t);
		}
	}
}

static void trieDestroyBranch(struct trie **pTrie, char *prefix, enum importance imp) {
	if (*prefix) {
		if (!*pTrie)
			return;
		trieDestroyBranch(trieGetChildPtr(*pTrie, *prefix), prefix+1, imp);
		if (!(*pTrie)->child && !(*pTrie)->scriptExpr)
			trieDestroy(pTrie, imp);
	} else {
		trieDestroy(pTrie, imp);
	}
}

static bool trieAdd(struct trie *trie, char *prefix, char *scriptExpr, int params, char *paramsFlags, enum importance importance) {
	struct trie *trie2=trie;
	for (char *p=prefix; *p; p++)
		trie=trieGetChildCreate(trie, *p);

	if (trie->child && params) // Ambiguous
		return false;
	if (trie->scriptExpr) // Already exists
		return false;

	trie=trie2;
	for (char *p=prefix; *p; p++) {
		trie=trieGetChild(trie, *p);
		if ((isDelim(*(p+1))) && (trie->importance<importance))
			trie->importance=importance;
	}
	trie->importance=importance;

	trie->scriptExpr=safeMalloc(strlen(scriptExpr)+1);
	strcpy(trie->scriptExpr, scriptExpr);
	if (!paramsFlags || !*paramsFlags)
		paramsFlags="-";
	trie->paramsFlags=safeMalloc(strlen(paramsFlags)+1);
	strcpy(trie->paramsFlags, paramsFlags);
	trie->params=params;
	return true;
}

static bool parseParam(char **dst, char **src, bool allRemaining, bool escaped) {
	while (*src && (**src==' '))
		(*src)++;
	bool quoted= (**src=='"') && !allRemaining;
	(*src)+=quoted;
	for (; **src; (*src)++) {
		if (!allRemaining && (**src==(quoted?'"':' '))) {
			(*src)++;
			**dst='\0';
			return false; // not opened
		}
		if (escaped && ((**src=='\\') || (**src=='"')))
			*(*dst)++ = '\\';
		*(*dst)++ = **src;
	}
	**dst='\0';
	return true; // opened
}

static char *trieTranslate(struct trie *trie, char *cmd) {
	static char *scriptExpr=0;

	for (; *cmd; cmd++) {
		struct trie *trie2=trie;
		trie=trieGetChild(trie, *cmd);
		if (!trie) {
			trie=trie2;
			break;
		}
	}

	if (!trie->scriptExpr)
		return 0;

	if ((!trie->params) && *cmd)
		return 0;

	utilStrRealloc(&scriptExpr, 0, strlen(trie->scriptExpr) + strlen(cmd) + 2);
	char *str=scriptExpr;
	char *format=trie->scriptExpr;
	char *flags=trie->paramsFlags;
	int params=trie->params;
	bool varArgs=(params<0);

	for (; *format; format++) {
		if (*format=='%') {
			if (*format+1=='%') {
				*str++='%';
				format++;
			} else {
				if (params>0)
					params--;
				do {
					if (params<0)
						params++;
					if (*flags!='-')
						*str++ = '"';
					parseParam(&str, &cmd, !params, (*flags!='-'));
					if (*flags!='-')
						*str++ = '"';
					if (varArgs && *cmd)
						*str++ = ',';
					if(!*++flags)
						flags--;
				} while (varArgs && *cmd);
			}
		} else {
			*str++ = *format;
		}
	}
	*str++='\0';

	return scriptExpr;
}

static void trieComplete(struct trie *trie, struct utilStrList **pLists, char **prefix, char **prefixEnd) {
	utilStrRealloc(prefix, prefixEnd, 1); // !!

	bool retCurrent=trie->scriptExpr;
	if (!retCurrent)
		for (struct trie *t=trie->child; t; t=t->sibling)
			if (isDelim(t->c)) {
				retCurrent=true;
				break;
			}

	if (retCurrent) {
		**prefixEnd='\0';
		utilStrListAddAfter(&(pLists[trie->importance]));
		utilStrRealloc(&(pLists[trie->importance])->str, 0, strlen(*prefix)+1);
		strcpy((pLists[trie->importance])->str, *prefix);
	}

	trie=trie->child;
	(*prefixEnd)++;
	for (; trie; trie=trie->sibling) {
		if (isDelim(trie->c))
			continue;
		*(*prefixEnd-1)=trie->c;
		trieComplete(trie, pLists, prefix, prefixEnd);
	}
	(*prefixEnd)--;
}

static struct utilStrList *trieGetSuffixes(struct trie *trie) {
	static char *completions=0;
	static char *completionsEnd=0;
	struct utilStrList *lists[3]={0,0,0};
	trieComplete(trie, lists, &completions, &completionsEnd);

	struct utilStrList *list=0;
	for (int imp=2; imp>=0; imp--) {
		if (lists[imp])
			while (lists[imp]->prev)
				lists[imp]=lists[imp]->prev;
		utilStrListMoveAfter(&list, lists[imp]);
	}

	if (list)
		while (list->prev)
			list=list->prev;
	return list;
}


// -- commands translation and completion --

static struct trie commands;

bool consoleCmdsAdd(char *prefix, char *scriptExpr, int params, char *paramsFlags, bool alias) {
	return trieAdd(
		&commands,
		prefix,
		scriptExpr,
		params,
		paramsFlags,
		(alias? builtinAliasCmd:builtinCmd)
	);
}

void consoleCmdsRmBranch(char *prefix) {
	if (*prefix) {
		struct trie **pTrie=trieGetChildPtr(&commands, *prefix);
		if (!*pTrie)
			return;
		trieDestroyBranch(pTrie, prefix+1, builtinCmd);
		trieDestroyBranch(pTrie, prefix+1, builtinAliasCmd);
	} else {
		for (struct trie **pt=&commands.child, *t2=*pt; *pt; (*pt!=t2) || (pt=&(*pt)->sibling), (t2=*pt)) {
			trieDestroy(pt, builtinCmd);
			trieDestroy(pt, builtinAliasCmd);
		}
	}
	DEBUG_CMDS(consoleCmdsTriePrint(&commands);)
}

char *consoleCmdsToScriptExpr(char *cmd) {
	return trieTranslate(&commands, cmd);
}

struct utilStrList *consoleCmdsComplete(char *prefix) {
	struct trie *trie=&commands;
	struct trie *trie2=trie;
	while (trie && *prefix) {
		trie2=trie;
		trie=trieGetChild(trie, *prefix);
		if (trie)
			prefix++;
	}
	if (trie && !trie->params) {
		return trieGetSuffixes(trie);
	} else {
		if (!trie)
			trie=trie2;
		if (!trie->scriptExpr)
			return NULL;

		char *flags=trie->paramsFlags;
		static char *param=0;
		bool opened=true;
		int params=trie->params;
		utilStrRealloc(&param, 0, strlen(prefix)+1);
		*param='\0';
		while (*prefix) {
			char *paramEnd=param;
			if (params>0)
				params--;
			opened=parseParam(&paramEnd, &prefix, !params, false);
			if (*prefix && *(flags+1))
				flags++;
		}
		if (opened)
			switch (*flags) {
				case 'p':
					return consoleCmdsPathComplete(param);
				case 'c':
					return consoleCmdsColorComplete(param, false);
				case 'C':
					return consoleCmdsColorComplete(param, true);
			}
	}
	return NULL;
}

void consoleCmdsUserAdd2(char *prefix, char *scriptExpr) {
	consoleCmdsUserAdd(prefix, scriptExpr, 0, NULL);
}
void consoleCmdsUserAdd3(char *prefix, char *scriptExpr, int params) {
	consoleCmdsUserAdd(prefix, scriptExpr, params, NULL);
}
void consoleCmdsUserAdd(char *prefix, char *scriptExpr, int params, char *paramsFlags) {
	if (!trieAdd(
		&commands,
		prefix,
		scriptExpr,
		params,
		paramsFlags,
		userCmd
	)) scriptThrowException("The command would be ambiguous");
}
void consoleCmdsUserRmBranch(char *prefix) {
	if (*prefix) {
		struct trie **pTrie=trieGetChildPtr(&commands, *prefix);
		if (!*pTrie)
			return;
		trieDestroyBranch(pTrie, prefix+1, userCmd);
	} else {
		for (struct trie **pt=&commands.child, *t2=*pt; *pt; (*pt!=t2) || (pt=&(*pt)->sibling), (t2=*pt))
			trieDestroy(pt, userCmd);
	}
}
void consoleCmdsUserRmAll() {
	consoleCmdsUserRmBranch("");
}

// -- path completion --

struct utilStrList *consoleCmdsPathComplete(char *prefix) {
	char *path=utilExpandPath(prefix);
	static char *tmp=0;
	utilStrRealloc(&tmp, 0, strlen(path)+1);
	prefix=tmp;
	if (prefix!=path) {
		strcpy(prefix, path);
		path=prefix;
	}
	prefix=utilFileNameFromPath(path);
	if (prefix==path)
		path=".";
	else
		*(prefix-1)='\0';
	int prefixLen=strlen(prefix);

	DIR *dir=opendir(path);
	if (!dir)
		return NULL;

	struct dirent *file;
	struct utilStrList *list=0;
	while (file=readdir(dir)) {
		if ((strncmp(prefix, file->d_name, prefixLen)==0) && (*prefix || (*file->d_name!='.'))) {
			utilStrListAddAfter(&list);
			utilStrRealloc(&list->str, 0, strlen(file->d_name)-prefixLen+1);
			strcpy(list->str, file->d_name+prefixLen);
		}
	}
	closedir(dir);

	if (list)
		while (list->prev)
			list=list->prev;

	return list;
}


// -- color translation and completion --

struct trie colors;
struct trie colorsWithAlpha;

struct utilStrList *consoleCmdsColorComplete(char *prefix, bool withAlphaChannel) {
	struct trie *trie=(withAlphaChannel? &colorsWithAlpha:&colors);
	struct utilStrList *list=0;
	int codeLen=0;
	while (*prefix) {
		if (((codeLen==0) &&   (*prefix=='#')) ||
		    ((codeLen> 0) && (((*prefix>='0') && (*prefix<='9')) ||
		                      ((*prefix>='a') && (*prefix<='f')) ||
		                      ((*prefix>='A') && (*prefix<='F'))))) {
			codeLen++;
		} else {
			codeLen=-1;
		}
		if (trie)
			trie=trieGetChild(trie, *prefix);
		else if (codeLen<0)
			break;
		prefix++;
	}
	if (codeLen>=0) {
		if (withAlphaChannel && (codeLen<=9)) {
			utilStrListAddAfter(&list);
			utilStrRealloc(&list->str, 0, 10);
			strcpy(list->str, "#AARRGGBB"+codeLen);
		} else if (codeLen<=7) {
			utilStrListAddAfter(&list);
			utilStrRealloc(&list->str, 0, 8);
			list->str[0]='\0';
			strcpy(list->str, "#RRGGBB"+codeLen);
		}
	}
	if (trie)
		utilStrListMoveAfter(&list, trieGetSuffixes(trie));
	while (list && list->prev)
		list=list->prev;
	return list;
}

#define normalizeColorChar(dst, src) \
	if (((src>='0') && (src<='9')) || \
	    ((src>='A') && (src<='F'))) \
		dst=src; \
	else if ((src>='a') && (src<='f')) \
		dst=src-'a'+'A'; \
	else { \
		ret=0; \
		break; \
	}

char *consoleCmdsColorNormalize(char *color) {
	char *ret=0;
	static char lastColor[8];
	if (*color=='#') {
		lastColor[0]='#';
		ret=lastColor;
		for (int i=1; i<7; i++)
			normalizeColorChar(lastColor[i], color[i]);
		lastColor[7]='\0';
		if (color[7])
			ret=0;
	} else {
		ret=trieTranslate(&colors, color);
		if (ret) {
			strcpy(lastColor, ret);
			ret=lastColor;
		}
	}
	if (!ret)
		scriptThrowException("Wrong color");
	return ret;
}

char *consoleCmdsColorANormalize(char *color) {
	char *ret=0;
	static char lastColor[10];
	if (*color=='#') {
		lastColor[0]='#';
		if (strlen(color)==7) {
			ret=lastColor;
			lastColor[1]='F';
			lastColor[2]='F';
			for (int i=1; i<7; i++)
				normalizeColorChar(lastColor[i+2], color[i]);
		} else if (strlen(color)==9) {
			ret=lastColor;
			for (int i=1; i<9; i++)
				normalizeColorChar(lastColor[i], color[i]);
		}
		lastColor[10]='\0';
	} else {
		ret=trieTranslate(&colorsWithAlpha, color);
		if (ret) {
			strcpy(lastColor, ret);
			ret=lastColor;
		}
	}
	if (!ret)
		scriptThrowException("Wrong color");
	return ret;
}

#undef normalizeColorChar

void consoleCmdsColorAdd(char *alias, char *color) {
	char *normalizedAlpha=consoleCmdsColorANormalize(color);
	if (!normalizedAlpha)
		return;
	char *normalized=     consoleCmdsColorNormalize(color);
	scriptCatchException();

	if (trieAdd(&colorsWithAlpha, alias, normalizedAlpha, 0, 0, userCmd)) {
		if (normalized) {
			if (trieAdd(&colors, alias, normalized, 0, 0, userCmd))
				return;
		} else {
			return;
		}
	}

	scriptThrowException("Color alias already exists");
}

void consoleCmdsColorRemoveAll() {
	for (struct trie **pt=&colors.child, *t2=*pt; *pt; (*pt!=t2) || (pt=&(*pt)->sibling), (t2=*pt))
		trieDestroy(pt, userCmd);
	for (struct trie **pt=&colorsWithAlpha.child, *t2=*pt; *pt; (*pt!=t2) || (pt=&(*pt)->sibling), (t2=*pt))
		trieDestroy(pt, userCmd);
}
