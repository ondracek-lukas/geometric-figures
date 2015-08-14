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
	extern void consoleCmdNew(int dim); // [SCRIPT_NAME: new]

	// Destroys opened space
	extern void consoleCmdClose(void); // [SCRIPT_NAME: close]

	// Rotates figure in the plane of the given axes by the given angle
	extern void consoleCmdRotate(float axis1, float axis2, float angle); // [SCRIPT_NAME: rotate]

	// Assigns/Unassigns rotation to the key
	extern void consoleCmdRmap(char *key, int axis1, int axis2); // [SCRIPT_NAME: rmap]
	extern void consoleCmdRunmap(char *key);                     // [SCRIPT_NAME: rmap]

	// Assigns/Unassigns command to the key
	extern void consoleCmdMap(char *key, char *cmd); // [SCRIPT_NAME: map]
	extern void consoleCmdUnmap(char *key);          // [SCRIPT_NAME: map]

	// Shows help, optionally to concrete command name
	extern void consoleCmdHelp(char *name); // [SCRIPT_NAME: help]

	// Shows history of commands
	extern void consoleCmdHistory(void); // [SCRIPT_NAME: history]

	// Exits application
	extern void consoleCmdQuit(void); // [SCRIPT_NAME: quit]

// consoleCmdSet.c:

	// Shows/edits settings; str should contain variable name and optionally new value
	extern void consoleCmdSet(char *str); // [SCRIPT_NAME: set]

// consoleCmdVertex.c:

	// Selects vertex of given index
	extern void consoleCmdVertexSelect(int index); // [SCRIPT_NAME: vertexSelect]

	// Selects next vertex
	extern void consoleCmdVertexNext(void); // [SCRIPT_NAME: vertexNext]

	// Selects previous vertex
	extern void consoleCmdVertexPrevious(void); // [SCRIPT_NAME: vertexPrevious]

	// Deselects vertex
	extern void consoleCmdVertexDeselect(void); // [SCRIPT_NAME: vertexDeselect]

	// Moves the selected vertex relatively to its position to the camera by given coordinates
	extern void consoleCmdVertexMove(int coordsCnt, float *coords); // [SCRIPT_NAME: vertexMove]

	// Adds new vertex at (optionally) given coordinates (relative to current figure rotation)
	extern void consoleCmdVertexAdd(int coordsCnt, float *coords); // [SCRIPT_NAME: vertexAdd]

	// Removes the selected vertex
	extern void consoleCmdVertexRm(void); // [SCRIPT_NAME: vertexRemove]

#endif
