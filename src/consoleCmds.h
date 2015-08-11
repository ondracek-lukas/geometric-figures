// Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

// consoleCmds manages register of existing commands and their translation into scripting language

#include <stdbool.h>

// Adds new command to the register.
// The command consists of the prefix followed (without space) by the given number of parameters,
// separated with spaces (ignored in double-quotes).
// scriptExpr is scripting language expression with % being replaced by parameters (use %% to escape),
// params-th % will be replaced with all remaining content.
// If params equals -1, the first % will be replaced with comma-separated list of all parameters.
extern bool consoleCmdsAdd(char *prefix, int params, char *scriptExpr);

// Converts command into scripting language expression.
// Returns NULL on error.
extern char *consoleCmdsToScriptExpr(char *cmd);
