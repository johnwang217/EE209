#ifndef _EXEC_H_
#define _EXEC_H_

#include "util.h"
#include "dynarray.h"

#define EXEC_FAIL -1
#define EXEC_SUCC 0

struct Command;

struct Command* buildCommand(DynArray_T oTokens);
void freeCommand(struct Command *c);
int execBuiltin(struct Command *c, enum BuiltinType btype);
int execCommand(struct Command *c);

#endif