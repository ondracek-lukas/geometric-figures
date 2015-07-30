// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// strings provides access to string resources in stringsData directory

// Each file in the stringsData directory begins with one line of sections and names in form:
	// section1:name1;section2:name2...
// Name of the file is not relevant (but has to be valid c identifier).
// Resources are identified only via any of given section and name pairs.
// Following lines forms the resource.
	// Only ASCII characters are allowed.
	// It can contain c escape sequencies and cannot contain TAB.
	// Special meaning has \b, which only moves cursor back by one character, allowing combining characters.
// Name of the file is not relevant.

#ifndef STRINGS_H
#define STRINGS_H

// Returns list of lines of the resource with the given name in the given section
// width and height will be set to the length and number of lines
extern struct utilStrList *stringsGet(char *section, char *name, int *width, int *height);

#endif
