// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

#include "strings.h"

#include <string.h>
#include <stdbool.h>

#include "util.h"

static bool stringsEq(char *section, char *name, const char *data[], int aliases);

#include "stringsData.c.tmp"

static bool stringsEq(char *section, char *name, const char *data[], int aliases) {
	int i, sectLen=strlen(section);
	for (i=0; i<aliases; i++)
		if ((strncmp(section, data[i], sectLen)==0) && (data[i][sectLen]==':') && (strcmp(name, data[i]+sectLen+1)==0))
			return 1;
	return 0;
}

struct utilStrList *stringsGet(char *section, char *name, int *width, int *height) {
	struct utilStrList *list=0;
	const char *data;
	char *lineEnd;
	int i, lineLen;
	data=stringsDataGet(section, name);
	if (!data)
		return 0;
	*width=0;
	*height=0;
	list=utilStrListAddAfter(list);
	lineLen=0;
	for (; *data; data++)
		switch(*data) {
			case '\n':
				utilStrRealloc(&list->str, 0, lineLen+1);
				list->str[lineLen]='\0';
				if (*width<(i=utilStrLineWidth(list->str)))
					*width=i;
				(*height)++;
				list=utilStrListAddAfter(list);
				lineLen=0;
				break;
			case '\t':
				data++;
				utilStrRealloc(&list->str, 0, lineLen+(unsigned char)*data);
				for (i=lineLen, lineLen+=(unsigned char)*data; i<lineLen; i++)
					list->str[i]=' ';
				break;
			default:
				utilStrRealloc(&list->str, 0, lineLen+1);
				list->str[lineLen++]=*data;
				break;
		}
	list=utilStrListRm(list);
	while (list) {
		if ((i=(*width)-utilStrLineWidth(list->str))>0) {
			lineEnd=strchr(list->str, '\0');
			utilStrRealloc(&list->str, &lineEnd, i+1);
			lineEnd[i]='\0';
			for (i--; i>=0; i--)
				lineEnd[i]=' ';
		}
		if (list->prev)
			list=list->prev;
		else
			break;
	}
	return list;
}
