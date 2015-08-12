// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmds.h"

#include <string.h>
#include <stdbool.h>

#include "util.h"

struct trie {
	char c;
	struct trie *sibling;
	struct trie *child;
	char *scriptExpr;
	char *paramsFlags;
	int params; // -1 for variable
};

static struct trie consoleCmdsRegister;

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

bool consoleCmdsAdd(char *prefix, int params, char *paramsFlags, char *scriptExpr) {
	struct trie *trie=&consoleCmdsRegister;
	for (; *prefix; prefix++)
		trie=trieGetChildCreate(trie, *prefix);

	if (trie->child && params) // Ambiguous
		return false;
	if (trie->scriptExpr) // Already exists
		return false;

	trie->scriptExpr=malloc(strlen(scriptExpr)+1);
	strcpy(trie->scriptExpr, scriptExpr);
	if (paramsFlags && *paramsFlags) {
		trie->paramsFlags=malloc(strlen(paramsFlags));
		strcpy(trie->paramsFlags, paramsFlags);
	} else {
		trie->paramsFlags="-";
	}
	trie->params=params;
	return true;
}

char *consoleCmdsToScriptExpr(char *cmd) {
	static char *scriptExpr=0;

	struct trie *trie=&consoleCmdsRegister;
	for (; cmd; cmd++) {
		struct trie *trie2=trie;
		trie=trieGetChild(trie, *cmd);
		if (!trie) {
			trie=trie2;
			break;
		}
	}

	if (!trie->scriptExpr)
		return 0;

	utilStrRealloc(&scriptExpr, 0, strlen(trie->scriptExpr) + strlen(cmd) + 2);
	char *str=scriptExpr;
	char *format=trie->scriptExpr;
	char *flags=trie->paramsFlags;
	unsigned params=trie->params;
	bool varArgs=(params==-1);

	for (; *format; format++) {
		if (*format=='%') {
			if (*format+1=='%') {
				*str++='%';
				format++;
			} else {
				if (params>0)
					params--;
				do {
					if (*flags=='s')
						*str++ = '"';
					while (cmd && (*cmd==' '))
						cmd++;
					bool quoted= (*cmd=='"');
					cmd+=quoted;
					for (; *cmd; cmd++) {
						if (params && (*cmd==(quoted?'"':' '))) {
							cmd++;
							break;
						}
						if (*cmd!='"')
							*str++ = *cmd;
					}
					if (*flags=='s')
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
