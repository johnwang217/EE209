#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "command.h"
#include "util.h"

struct Command* buildCommand(DynArray_T oTokens) {
  assert(oTokens != NULL);

  struct Command *input;
  input = (struct Command*)calloc(1, sizeof(struct Command));
  input->maxLength = DynArray_getLength(oTokens) + 1;
  input->arguments = 
    (char**)calloc(1, (input->maxLength)*sizeof(char*));
  input->pipedArgs = 
    (char**)calloc(1, (input->maxLength)*sizeof(char*));
  if (input == NULL || input->arguments == NULL || input->pipedArgs == NULL) {
    return NULL;
  }
  input->argLength = 0;
  input->pipeLength = 0;

  struct Token *t;
  int pexist = FALSE;
  for (int i = 0; i < DynArray_getLength(oTokens); i++) {
    t = DynArray_get(oTokens, i);
    enum TokenType type = t->eType;
    if (pexist == TRUE && type == TOKEN_WORD) type = TOKEN_PIPE;  
    switch (type) {
      case TOKEN_WORD:
        input->arguments[input->argLength] = t->pcValue;
        input->argLength++;
        break;
      case TOKEN_REDIN:
        t = DynArray_get(oTokens, ++i);
        input->redin = t->pcValue;
        break;
      case TOKEN_REDOUT:
        t = DynArray_get(oTokens, ++i);
        input->redout = t->pcValue;
        break;
      case TOKEN_PIPE:
        if (pexist == FALSE) {
          pexist = TRUE;
          for (int j = 0; j < input->argLength; j++) {
            input->pipedArgs[j] = input->arguments[j];
          }
          input->pipeLength = input->argLength;
          input->argLength = 0;
          memset(input->arguments, 0, (input->maxLength)*sizeof(char *));
        }
        input->pipedArgs[input->pipeLength] = t->pcValue;
        input->pipeLength++;
        break;
      case TOKEN_BG:
        break;
    }
  }
  return input;
}

void freeCommand(struct Command *c) {
  if (c == NULL) return;

  free(c->arguments);
  free(c->pipedArgs);
  free(c);
  return;
}