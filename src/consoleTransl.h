// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// consoleTransl manages register of existing commands and their translation into scripting language

#include <stdbool.h>

// Adds new command to the register.
// The command consists of the prefix followed (without space) by the given number of parameters,
// separated with spaces (ignored in double-quotes, all double-quotes are removed).
// scriptExpr is scripting language expression with % being replaced by parameters (use %% to escape),
// abs(params)-th % will be replaced with all remaining content.
// If params is negative, the first % will be replaced with comma-separated list of all parameters.
// paramsFlags should contain one char for each %;
// if it is shorter, the last character will be repeated;
// if it is NULL or "", - character will be repeated.
//     -:  parameter will be used "as is" as literal
//     s:  parameter will be surrounded by double-quotes as string literal
//     p:  also string but treated as path with autocompletion
//    c/C: also string but treated as color with autocompletion, capital C allows transparency
extern bool consoleTranslAdd(char *prefix, char *scriptExpr, int params, char *paramsFlags, bool alias);

// Removes all commands with the given prefix.
extern void consoleTranslRmBranch(char *prefix);

// Converts command into scripting language expression.
// Returns NULL on error.
extern char *consoleTranslToScriptExpr(char *cmd);

// Returns possible completions to given command prefix
extern struct utilStrList *consoleTranslComplete(char *prefix);

// User versions, throws exceptions using script module
void consoleTranslUserAdd2(char *prefix, char *scriptExpr);                               // [SCRIPT_NAME: addCommand]
void consoleTranslUserAdd3(char *prefix, char *scriptExpr, int params);                   // [SCRIPT_NAME: addCommand]
void consoleTranslUserAdd(char *prefix, char *scriptExpr, int params, char *paramsFlags); // [SCRIPT_NAME: addCommand]
void consoleTranslUserRmBranch(char *prefix);                                             // [SCRIPT_NAME: removeCommands]
void consoleTranslUserRmAll();                                                            // [SCRIPT_NAME: removeCommands]


// Returns possible completions to given path
extern struct utilStrList *consoleTranslPathComplete(char *prefix);


// Returns possible completions to given color (+ mask #[AA]RRGGBB)
extern struct utilStrList *consoleTranslColorComplete(char *prefix, bool withAlphaChannel);

// The following functions throws exceptions using script module:

	// Adds new color name
	void consoleTranslColorAdd(char *alias, char *color);  // [SCRIPT_NAME: addColorAlias]

	// Removes all named colors
	void consoleTranslColorRemoveAll();                    // [SCRIPT_NAME: resetColorAliases]

	// Returns the color in a normalized form #RRGGBB, resp. #AARRGGBB
	char *consoleTranslColorNormalize(char *color);        // [SCRIPT_NAME: normalizeColor]
	char *consoleTranslColorANormalize(char *color);       // [SCRIPT_NAME: normalizeColorAlpha]
