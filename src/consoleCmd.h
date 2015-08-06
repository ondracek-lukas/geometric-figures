// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// consoleCmd contains commands to be called from application command line

#ifndef CONSOLE_CMD_H
#define CONSOLE_CMD_H

// consoleCmd.c:

	// Reads commands from file
	extern void consoleCmdSource(char *path); // [SCRIPT_NAME: source]

	// Opens figure from file
	extern void consoleCmdOpen(char *path); // [SCRIPT_NAME: open]

	// Writes figure to file
	extern void consoleCmdWrite(char *path); // [SCRIPT_NAME: write]

	// Creates new space, str should contain number of dimensions
	extern void consoleCmdNew(char *str); // [SCRIPT_NAME: new]

	// Rotates figure, str should contain two axes and angle
	extern void consoleCmdRotate(char *str); // [SCRIPT_NAME: rotate]

	// Assigns rotation to a key; str should contain key, and two axes or nothing (to cancel)
	extern void consoleCmdRmap(char *str); // [SCRIPT_NAME: rmap]

	// Assigns command to a key; str should contain key, and command or nothing (to cancel)
	extern void consoleCmdMap(char *str); // [SCRIPT_NAME: map]

	// Shows help, optionally to concrete command name
	extern void consoleCmdHelp(char *name); // [SCRIPT_NAME: help]

	// Shows history of commands
	extern void consoleCmdHistory(void); // [SCRIPT_NAME: history]

// consoleCmdSet.c:

	// Shows/edits settings; str should contain variable name and optionally new value
	extern void consoleCmdSet(char *str); // [SCRIPT_NAME: set]

// consoleCmdVertex.c:

	// Selects vertex of given index
	extern void consoleCmdVertexSelect(char *param); // [SCRIPT_NAME: vertexSelect]

	// Selects next vertex
	extern void consoleCmdVertexNext(void); // [SCRIPT_NAME: vertexNext]

	// Selects previous vertex
	extern void consoleCmdVertexPrevious(void); // [SCRIPT_NAME: vertexPrevious]

	// Deselects vertex
	extern void consoleCmdVertexDeselect(void); // [SCRIPT_NAME: vertexDeselect]

	// Moves the selected vertex relatively to its position to the camera by given coordinates
	extern void consoleCmdVertexMove(char *params); // [SCRIPT_NAME: vertexMove]

	// Adds new vertex at (optionally) given coordinates (relative to current figure rotation)
	extern void consoleCmdVertexAdd(char *params); // [SCRIPT_NAME: vertexAdd]

	// Removes the selected vertex
	extern void consoleCmdVertexRm(void); // [SCRIPT_NAME: vertexRm]

#endif
