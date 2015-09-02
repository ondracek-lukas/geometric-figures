// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// consoleCmdSet manages getters and setters for variables settable by user
// This file is intended to be included from consoleCmd.h,
// don't include it directly.

#include <stdbool.h>

// Updates list of variables in consoleCmds module
extern void consoleCmdSetUpdateCmds();

// See ':help set varname' for description of consoleCmdGetVarname or consoleCmdSetVarname

char *consoleCmdGetBackground();                      // [SCRIPT_NAME: get_background]
void consoleCmdSetBackground(char *color);            // [SCRIPT_NAME: set_background]

float consoleCmdGetCampos(int index);                 // [SCRIPT_NAME: get_campos]
void consoleCmdSetCampos(int index, float value);     // [SCRIPT_NAME: set_campos]

float consoleCmdGetCamposl(int index);                // [SCRIPT_NAME: get_camposl]
void consoleCmdSetCamposl(int index, float value);    // [SCRIPT_NAME: set_camposl]

bool consoleCmdGetConvexhull();                       // [SCRIPT_NAME: get_convexhull]
void consoleCmdSetConvexhull(bool value);             // [SCRIPT_NAME: set_convexhull]

int consoleCmdGetDimen();                             // [SCRIPT_NAME: get_dimen]
void consoleCmdSetDimen(bool value);                  // [SCRIPT_NAME: set_dimen]

float consoleCmdGetEdgesize();                        // [SCRIPT_NAME: get_edgesize]
void consoleCmdSetEdgesize(float value);              // [SCRIPT_NAME: set_edgesize]

char *consoleCmdGetFacecolor();                       // [SCRIPT_NAME: get_facecolor]
void consoleCmdSetFacecolor(char *color);             // [SCRIPT_NAME: set_facecolor]

bool consoleCmdGetGrabmouse();                        // [SCRIPT_NAME: get_grabmouse]
void consoleCmdSetGrabmouse(bool value);              // [SCRIPT_NAME: set_grabmouse]

int consoleCmdGetHistory();                           // [SCRIPT_NAME: get_history]
void consoleCmdSetHistory(int value);                 // [SCRIPT_NAME: set_history]

int consoleCmdGetMaxfps();                            // [SCRIPT_NAME: get_maxfps]
void consoleCmdSetMaxfps(int value);                  // [SCRIPT_NAME: set_maxfps]

float consoleCmdGetMousesens();                       // [SCRIPT_NAME: get_mousesens]
void consoleCmdSetMousesens(float value);             // [SCRIPT_NAME: set_mousesens]

bool consoleCmdGetPyexpr();                           // [SCRIPT_NAME: get_pyexpr]
void consoleCmdSetPyexpr(bool value);                 // [SCRIPT_NAME: set_pyexpr]

char *consoleCmdGetSelvertcolor();                    // [SCRIPT_NAME: get_selvertcolor]
void consoleCmdSetSelvertcolor(char *color);          // [SCRIPT_NAME: set_selvertcolor]

float consoleCmdGetSelvertsize();                     // [SCRIPT_NAME: get_selvertsize]
void consoleCmdSetSelvertsize(float value);           // [SCRIPT_NAME: set_selvertsize]

char *consoleCmdGetSpacecolor(int index);             // [SCRIPT_NAME: get_spacecolor]
void consoleCmdSetSpacecolor(int index, char *color); // [SCRIPT_NAME: set_spacecolor]

float consoleCmdGetSpeed();                           // [SCRIPT_NAME: get_speed]
void consoleCmdSetSpeed(float value);                 // [SCRIPT_NAME: set_speed]

bool consoleCmdGetStdoutpyexpr();                     // [SCRIPT_NAME: get_stdoutpyexpr]
void consoleCmdSetStdoutpyexpr(bool value);           // [SCRIPT_NAME: set_stdoutpyexpr]

float consoleCmdGetVertsize();                        // [SCRIPT_NAME: get_vertsize]
void consoleCmdSetVertsize(float value);              // [SCRIPT_NAME: set_vertsize]
