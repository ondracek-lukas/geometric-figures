// Geometric Figures  Copyright (C) 2015--2016  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// consoleCmdSet manages getters and setters for variables settable by user
// This file is intended to be included from consoleCmd.h,
// don't include it directly.

#include <stdbool.h>

// Updates list of variables in consoleTransl module
extern void consoleCmdSetUpdateCmds();

// See ':help set varname' for description of consoleCmdGetVarname or consoleCmdSetVarname

char *consoleCmdGetBackground();                      // [SCRIPT_NAME: get_background]
void consoleCmdSetBackground(char *color);            // [SCRIPT_NAME: set_background]

double consoleCmdGetCampos(int index);                 // [SCRIPT_NAME: get_campos]
void consoleCmdSetCampos(int index, double value);     // [SCRIPT_NAME: set_campos]

double consoleCmdGetCamposl(int index);                // [SCRIPT_NAME: get_camposl]
void consoleCmdSetCamposl(int index, double value);    // [SCRIPT_NAME: set_camposl]

bool consoleCmdGetConvexhull();                       // [SCRIPT_NAME: get_convexhull]
void consoleCmdSetConvexhull(bool value);             // [SCRIPT_NAME: set_convexhull]

int consoleCmdGetDimen();                             // [SCRIPT_NAME: get_dimen]
void consoleCmdSetDimen(bool value);                  // [SCRIPT_NAME: set_dimen]

double consoleCmdGetEdgesize();                        // [SCRIPT_NAME: get_edgesize]
void consoleCmdSetEdgesize(double value);              // [SCRIPT_NAME: set_edgesize]

char *consoleCmdGetFacecolor();                       // [SCRIPT_NAME: get_facecolor]
void consoleCmdSetFacecolor(char *color);             // [SCRIPT_NAME: set_facecolor]

bool consoleCmdGetGrabmouse();                        // [SCRIPT_NAME: get_grabmouse]
void consoleCmdSetGrabmouse(bool value);              // [SCRIPT_NAME: set_grabmouse]

int consoleCmdGetHistory();                           // [SCRIPT_NAME: get_history]
void consoleCmdSetHistory(int value);                 // [SCRIPT_NAME: set_history]

int consoleCmdGetMaxfps();                            // [SCRIPT_NAME: get_maxfps]
void consoleCmdSetMaxfps(int value);                  // [SCRIPT_NAME: set_maxfps]

double consoleCmdGetMousesens();                       // [SCRIPT_NAME: get_mousesens]
void consoleCmdSetMousesens(double value);             // [SCRIPT_NAME: set_mousesens]

bool consoleCmdGetPyexpr();                           // [SCRIPT_NAME: get_pyexpr]
void consoleCmdSetPyexpr(bool value);                 // [SCRIPT_NAME: set_pyexpr]

char *consoleCmdGetSelvertcolor();                    // [SCRIPT_NAME: get_selvertcolor]
void consoleCmdSetSelvertcolor(char *color);          // [SCRIPT_NAME: set_selvertcolor]

double consoleCmdGetSelvertsize();                     // [SCRIPT_NAME: get_selvertsize]
void consoleCmdSetSelvertsize(double value);           // [SCRIPT_NAME: set_selvertsize]

char *consoleCmdGetSpacecolor(int index);             // [SCRIPT_NAME: get_spacecolor]
void consoleCmdSetSpacecolor(int index, char *color); // [SCRIPT_NAME: set_spacecolor]

double consoleCmdGetSpeed();                           // [SCRIPT_NAME: get_speed]
void consoleCmdSetSpeed(double value);                 // [SCRIPT_NAME: set_speed]

bool consoleCmdGetStdoutpyexpr();                     // [SCRIPT_NAME: get_stdoutpyexpr]
void consoleCmdSetStdoutpyexpr(bool value);           // [SCRIPT_NAME: set_stdoutpyexpr]

double consoleCmdGetVertsize();                        // [SCRIPT_NAME: get_vertsize]
void consoleCmdSetVertsize(double value);              // [SCRIPT_NAME: set_vertsize]
