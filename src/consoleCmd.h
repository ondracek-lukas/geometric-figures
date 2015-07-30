// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// consoleCmd contains commands to be called from application command line

#ifndef CONSOLE_CMD_H
#define CONSOLE_CMD_H

// consoleCmd.c:

	// Reads commands from file
	extern void consoleCmdSource(char *path);

	// Opens figure from file
	extern void consoleCmdOpen(char *path);

	// Writes figure to file
	extern void consoleCmdWrite(char *path);

	// Creates new space, str should contain number of dimensions
	extern void consoleCmdNew(char *str);

	// Rotates figure, str should contain two axes and angle
	extern void consoleCmdRotate(char *str);

	// Assigns rotation to a key; str should contain key, and two axes or nothing (to cancel)
	extern void consoleCmdRmap(char *str);

	// Assigns command to a key; str should contain key, and command or nothing (to cancel)
	extern void consoleCmdMap(char *str);

	// Shows help, optionally to concrete command name
	extern void consoleCmdHelp(char *name);

	// Shows history of commands
	extern void consoleCmdHistory(void);

// consoleCmdSet.c:

	// Shows/edits settings; str should contain variable name and optionally new value
	extern void consoleCmdSet(char *str);

// consoleCmdVertex.c:

	// Selects vertex of given index
	extern void consoleCmdVertexSelect(char *param);

	// Selects next vertex
	extern void consoleCmdVertexNext(void);

	// Selects previous vertex
	extern void consoleCmdVertexPrevious(void);

	// Deselects vertex
	extern void consoleCmdVertexDeselect(void);

	// Moves the selected vertex relatively to its position to the camera by given coordinates
	extern void consoleCmdVertexMove(char *params);

	// Adds new vertex at (optionally) given coordinates (relative to current figure rotation)
	extern void consoleCmdVertexAdd(char *params);

	// Removes the selected vertex
	extern void consoleCmdVertexRm(void);

#endif
