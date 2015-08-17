// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "consoleCmds.h"

#include <string.h>
#include <stdbool.h>

#include "util.h"
#include "debug.h"
#include "safe.h"

struct trie {
	char c;
	struct trie *sibling;
	struct trie *child;
	char *scriptExpr;
	char *paramsFlags;
	int params; // -1 for variable
};

static struct trie consoleCmdsRegister;

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

static void trieDestroy(struct trie **pTrie) {
	while ((*pTrie)->child)
		trieDestroy(&(*pTrie)->child);
	struct trie *trie=*pTrie;
	(*pTrie)=trie->sibling;
	free(trie->scriptExpr);
	free(trie->paramsFlags);
	free(trie);
}

static void trieDestroyBranch(struct trie **pTrie, char *prefix) {
	if (*prefix) {
		if (!*pTrie)
			return;
		trieDestroyBranch(trieGetChildPtr(*pTrie, *prefix), prefix+1);
		if (!(*pTrie)->child)
			trieDestroy(pTrie);
	} else {
		trieDestroy(pTrie);
	}
}

bool consoleCmdsAdd(char *prefix, int params, char *paramsFlags, char *scriptExpr) {
	struct trie *trie=&consoleCmdsRegister;
	for (; *prefix; prefix++)
		trie=trieGetChildCreate(trie, *prefix);

	if (trie->child && params) // Ambiguous
		return false;
	if (trie->scriptExpr) // Already exists
		return false;

	trie->scriptExpr=safeMalloc(strlen(scriptExpr)+1);
	strcpy(trie->scriptExpr, scriptExpr);
	if (!paramsFlags || !*paramsFlags)
		paramsFlags="-";
	trie->paramsFlags=safeMalloc(strlen(paramsFlags)+1);
	strcpy(trie->paramsFlags, paramsFlags);
	trie->params=params;
	DEBUG_CMDS(consoleCmdsTriePrint(&consoleCmdsRegister);)
	return true;
}

void consoleCmdsRmBranch(char *prefix) {
	if (*prefix) {
		struct trie **pTrie=trieGetChildPtr(&consoleCmdsRegister, *prefix);
		if (!*pTrie)
			return;
		trieDestroyBranch(pTrie, prefix+1);
	} else {
		while (consoleCmdsRegister.child)
			trieDestroy(&consoleCmdsRegister.child);
	}
	DEBUG_CMDS(consoleCmdsTriePrint(&consoleCmdsRegister);)
}

char *consoleCmdsToScriptExpr(char *cmd) {
	static char *scriptExpr=0;

	struct trie *trie=&consoleCmdsRegister;
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
