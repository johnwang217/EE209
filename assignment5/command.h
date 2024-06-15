#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "dynarray.h"

struct Command 
{
  char **arguments;
  
  char *redin;
  
  char *redout;
  
  char **pipes;
  
  int arg_index;
  
  int pip_index;
};

struct Command* buildCommand(DynArray_T oTokens);
void freeCommand(struct Command *c);

#endif