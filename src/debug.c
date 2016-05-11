// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "debug.h"

#ifdef DEBUG

#include <stdio.h>

#include "convex.h"
#include "convexFig.h"
#include "convexSpace.h"
#include "script.h"
#include "scriptFigure.h"
#include "figure.h"

void convexFigPrint() { // mark unsafe
	printf("digraph{");
	convexFigMarkReset(convexFigMarkIdLayer);
	convexFigPrintR(convexFigure, "convexFigure");
	convexFigPrintR(convexFreeVertices, "convexFreeVertices");
	printf("}\n");
}

void convexFigPrintR(struct convexFigList *list, char *parent) {
	char str[50];
	while (list) {
		if (list->fig->space->dim>=0) {
			sprintf(str, "%dD - %02d - %08x",list->fig->space->dim,list->fig->index,list->fig->hash);
			printf("\"%s\"->\"%s\";", parent, str);
			if (!convexFigMarkGet(list->fig, convexFigMarkIdLayer)) {
				convexFigPrintR(list->fig->boundary, str);
				convexFigMarkSet(list->fig, convexFigMarkIdLayer);
			}
		}
		list=list->next;
	}
}


// consoleTransl.c:
struct trie {
	char c;
	struct trie *sibling;
	struct trie *child;
	char *scriptExpr;
	char *paramsFlags;
	int params; // -1 for variable
};

void triePrint(struct trie *trie, int indent) {
	if (!trie)
		return;
	printf("%c%c", (trie->c?trie->c:'_'), (trie->scriptExpr || trie->paramsFlags || trie->params? '*':'-'));
	triePrint(trie->child, indent+2);
	while (trie=trie->sibling) {
		printf("\n%*c%c%c", indent, '|', (trie->c?trie->c:'_'), (trie->scriptExpr || trie->paramsFlags || trie->params? '*':'-'));
		triePrint(trie->child, indent+2);
	}
}
void consoleTranslTriePrint(struct trie *trie) {
	printf("--- TRIE START ---\n--");
	triePrint(trie, 2);
	printf("\n--- TRIE  END  ---\n");
}


// Convex hull progress

void debugProgrStart(struct figureData *figure, struct convexFig ***shadow) {
	char *figExpr=scriptFigureToPythonExpr(figure);
	printf("import gf\nimport debugPlayback\n");
	if (figExpr) {
		printf("debugPlayback.open(%s, {", figExpr);
		for (int i=0; i<=figure->dim; i++) {
			for (int j=0; j<figure->count[i]; j++) {
				if (shadow[i][j])
					printf("%u:(%d,%d),", shadow[i][j]->hash, i, j);
			}
		}
		printf("})\n");
	} else {
		printf("debugPlayback.figureOpen(None, None)\n");
	}
}

void debugProgrAttach(struct convexFig *parent, struct convexFig *child) {
	printf("debugPlayback.attach(%u,%u)\n", parent->hash, child->hash);
}

void debugProgrDetach(struct convexFig *parent, struct convexFig *child) {
	printf("debugPlayback.detach(%u,%u)\n", parent->hash, child->hash);
}

void debugProgrHashChange(unsigned int oldHash, unsigned int newHash) {
	printf("debugPlayback.rename(%u,%u)\n", oldHash, newHash);
}

#endif
