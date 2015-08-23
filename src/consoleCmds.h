// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// consoleCmds manages register of existing commands and their translation into scripting language

#include <stdbool.h>

// Adds new command to the register.
// The command consists of the prefix followed (without space) by the given number of parameters,
// separated with spaces (ignored in double-quotes, all double-quotes are removed).
// scriptExpr is scripting language expression with % being replaced by parameters (use %% to escape),
// params-th % will be replaced with all remaining content.
// If params equals -1, the first % will be replaced with comma-separated list of all parameters.
// paramsFlags should contain one char for each %;
// if it is shorter, the last character will be repeated;
// if it is NULL or "", - character will be repeated.
//     -:  parameter will be used "as is" as literal
//     s:  parameter will be surrounded by double-quotes as string literal
//     p:  also string but treated as path with autocompletion
//    c/C: also string but treated as color with autocompletion, capital C allows transparency
extern bool consoleCmdsAdd(char *prefix, char *scriptExpr, int params, char *paramsFlags, bool alias);

// Removes all commands with the given prefix.
extern void consoleCmdsRmBranch(char *prefix);

// Converts command into scripting language expression.
// Returns NULL on error.
extern char *consoleCmdsToScriptExpr(char *cmd);

// Returns possible completions to given command prefix
extern struct utilStrList *consoleCmdsComplete(char *prefix);

// Returns possible completions to given path
extern struct utilStrList *consoleCmdsPathComplete(char *prefix);

// Adds new color name (colors without alpha are accessible even as colors with alpha)
extern bool consoleCmdsAddColor(char *colorName, char *colorCode, bool hasAlphaChannel);

// Returns possible completions to given color (+ mask #[AA]RRGGBB)
extern struct utilStrList *consoleCmdsColorComplete(char *prefix, bool withAlphaChannel);

// Translates color name to color code
extern char *consoleCmdsColorNameToCode(char *name, bool withAlphaChannel);
