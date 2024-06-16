#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "dynarray.h"

struct Command 
{
  char **arguments;

  char **pipedArgs;
  
  char *redin;
  
  char *redout;
  
  int argLength;
  
  int pipeLength;

  int maxLength;
};

struct Command* buildCommand(DynArray_T oTokens);
void freeCommand(struct Command *c);

#endif