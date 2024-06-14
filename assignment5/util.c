#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "dynarray.h"
#include "util.h"

void
errorPrint(char *input, enum PrintMode mode) {
  static char *ishname = NULL;

  if (mode == SETUP)
    ishname = input;
  else {
    if (ishname == NULL)
      fprintf(stderr, "[WARN] Shell name is not set. Please fix this bug in main function\n");
    if (mode == PERROR) {
      if (input == NULL)
        fprintf(stderr, "%s: %s\n", ishname, strerror(errno));
      else
        fprintf(stderr, "%s: %s\n", input, strerror(errno));
    }
    else if (mode == FPRINTF)
      fprintf(stderr, "%s: %s\n", ishname, input);
    else if( mode == ALIAS)
      fprintf(stderr, "%s: alias: %s: not found\n", ishname, input);
    else
      fprintf(stderr, "mode %d not supported in errorPrint\n", mode);
    }
}

enum BuiltinType
checkBuiltin(struct Token *t) {
  /* Check null input before using string functions  */
  assert(t);
  assert(t->pcValue);

  if (strncmp(t->pcValue, "cd", 2) == 0 && strlen(t->pcValue) == 2)
    return B_CD;
  if (strncmp(t->pcValue, "fg", 2) == 0 && strlen(t->pcValue) == 2)
    return B_FG;
  if (strncmp(t->pcValue, "exit", 4) == 0 && strlen(t->pcValue) == 4)
    return B_EXIT;
  else if (strncmp(t->pcValue, "setenv", 6) == 0 && strlen(t->pcValue) == 6)
    return B_SETENV;
  else if (strncmp(t->pcValue, "unsetenv", 8) == 0 && strlen(t->pcValue) == 8)
    return B_USETENV;
  else if (strncmp(t->pcValue, "alias" , 5) == 0 && strlen(t->pcValue) == 5) 
    return B_ALIAS;
  else
    return NORMAL;
}

int
countPipe(DynArray_T oTokens) {
  int cnt = 0, i;
  struct Token *t;

  for (i = 0; i < DynArray_getLength(oTokens); i++) {
    t = DynArray_get(oTokens, i);
    if (t->eType == TOKEN_PIPE)
      cnt++;
  }

  return cnt;
}

/* Check background Command */
int
checkBG(DynArray_T oTokens) {
  int i;
  struct Token *t;

  for (i = 0; i < DynArray_getLength(oTokens); i++) {
    t = DynArray_get(oTokens, i);
    if (t->eType == TOKEN_BG)
      return 1;
  }
  return 0;
}

const char* specialTokenToStr(struct Token* psToken) {
  switch(psToken->eType) {
    case TOKEN_PIPE:
      return "TOKEN_PIPE(|)";
      break;
    case TOKEN_REDIN:
      return "TOKEN_REDIRECTION_IN(<)";
      break;
    case TOKEN_REDOUT:
      return "TOKEN_REDIRECTION_OUT(>)";
      break;
    case TOKEN_BG:
      return "TOKEN_BACKGROUND(&)";
      break;
    case TOKEN_WORD:
      /* This should not be called with TOKEN_WORD */
    default:
      assert(0 && "Unreachable");
      return NULL;
  }
}

void
dumpLex(DynArray_T oTokens) {
  if (getenv("DEBUG") != NULL) {
    int i;
    struct Token *t;

    for (i = 0; i < DynArray_getLength(oTokens); i++) {
      t = DynArray_get(oTokens, i);
      if (t->pcValue == NULL)
        fprintf(stderr, "[%d] %s\n", i, specialTokenToStr(t));
      else
        fprintf(stderr, "[%d] TOKEN_WORD(\"%s\")\n", i, t->pcValue);
    }
  }
}

struct Command*
buildCommand(DynArray_T oTokens) {
  struct Command *input = calloc(1, sizeof(struct Command));
  input->arguments = calloc(1, (oTokens->iLength + 1)*sizeof(char *));
  input->pipes = calloc(1, (oTokens->iLength + 1)*sizeof(char *));
  // add error handling "not enough memory" return NULL
  // remember to free all even if alloc fails after successful allocation of first alloc

  struct Token *t;
  int arg_index = 0, pip_index = 0;
  int pexist = FALSE;
  for (int i = 0; i < oTokens->iLength; i++) {
    t = DynArray_get(oTokens, i);
    enum TokenType type = t->eType;
    if (pexist == TRUE && type == TOKEN_WORD) type = TOKEN_PIPE;  
    switch (type) {
      case TOKEN_WORD:
        input->arguments[arg_index] = t->pcValue;
        arg_index++;
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
        if (pexist == FALSE) pexist = TRUE;
        input->pipes[pip_index] = t->pcValue;
        pip_index++;
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

void freeArrayTokens(DynArray_T oTokens) {
   if (oTokens == NULL) return;
   
   struct Token *t;
   for (int i = 0; i < oTokens->iLength; i++) {
    t = DynArray_get(oTokens, i);
    freeToken(t, NULL);
   }
}