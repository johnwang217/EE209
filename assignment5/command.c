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
  input->arguments = 
    (char**)calloc(1, (DynArray_getLength(oTokens) + 1)*sizeof(char*));
  input->pipes = 
    (char**)calloc(1, (DynArray_getLength(oTokens) + 1)*sizeof(char*));
  if (input == NULL || input->arguments == NULL || input->pipes == NULL) {
    return NULL;
  }
  input->arg_index = 0;
  input->pip_index = 0;

  struct Token *t;
  int pexist = FALSE;
  for (int i = 0; i < DynArray_getLength(oTokens); i++) {
    t = DynArray_get(oTokens, i);
    enum TokenType type = t->eType;
    if (pexist == TRUE && type == TOKEN_WORD) type = TOKEN_PIPE;  
    switch (type) {
      case TOKEN_WORD:
        input->arguments[input->arg_index] = t->pcValue;
        input->arg_index++;
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
          for (int j = 0; j < input->arg_index; j++) {
            input->pipes[j] = input->arguments[j];
          }
          input->pip_index = input->arg_index;
          input->arg_index = 0;
          memset(input->arguments, 0, (DynArray_getLength(oTokens) + 1)*sizeof(char *));
        }
        input->pipes[input->pip_index] = t->pcValue;
        input->pip_index++;
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
  free(c->pipes);
  free(c);
  return;
}