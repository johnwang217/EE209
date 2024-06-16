/*--------------------------------------------------------------------*/
/* exec.c                                                             */
/* Author: Wang Jonghyuk                                              */
/* Student ID: 20220425                                               */
/* Description: defines functions for executing shell commands        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#include "token.h"
#include "exec.h"

extern FILE* input_fp;

/*--------------------------------------------------------------------*/

/* Command consists of arrays of token values split by their types and
    their respective lengths. */

struct Command 
{
  /* Array that holds argument values for execution */
  char **arguments;

  /* Array that holds argument values of pending commands waiting for
      input from pipe for execution. Each commands are seperated by 
      NULL value */
  char **pipedArgs;
  
  /* String that holds name of file for standard input redirection */
  char *redin;
  
  /* String that holds name of file for standard output redirection */
  char *redout;
  
  /* Number of elements in argument array */
  int argLength;
  
  /* Number of elements in pending argument array */
  int pipeLength;

  /* Max number of elements arrays can hold */
  int maxLength;
};

/*--------------------------------------------------------------------*/

/* Return a new Command built with values from oTokens. Returns a NULL
    pointer if it couldn't allocate memory. */
struct Command* buildCommand(DynArray_T oTokens) {
  assert(oTokens != NULL);

  struct Command *input;
  input = (struct Command*)calloc(1, sizeof(struct Command));
  input->maxLength = DynArray_getLength(oTokens) + 1;
  input->arguments = 
    (char**)calloc(1, (input->maxLength)*sizeof(char*));
  input->pipedArgs = 
    (char**)calloc(1, (input->maxLength)*sizeof(char*));
  if (input == NULL || input->arguments == NULL 
    || input->pipedArgs == NULL) {
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
          memset(input->arguments, 0, (input->maxLength)*sizeof(char*));
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

/*--------------------------------------------------------------------*/

/* frees memory allocated to Command c */
void freeCommand(struct Command *c) {
  if (c == NULL) return;

  free(c->arguments);
  free(c->pipedArgs);
  free(c);
  return;
}

/*--------------------------------------------------------------------*/

/* executes built-in command of type 'btype' with arguments from 'c'.
    When error occurs, prints out appropriate messages to
    stderr and returns EXEC_FAIL . If no error, returns EXEC_SUCC. */
int execBuiltin (struct Command *c, enum BuiltinType btype) {
  assert(c != NULL);

  if (btype == B_SETENV) {
    if ((c->argLength != 2 && c->argLength != 3) 
      || c->redin != NULL || c->redout != NULL) {
      errorPrint("setenv takes one or two parameters", FPRINTF);
      return EXEC_FAIL;
    }
    if (c->arguments[2] == NULL) c->arguments[2] = "";
    if(setenv(c->arguments[1], c->arguments[2], 1) < 0) {
      errorPrint (NULL, PERROR);
      return EXEC_FAIL;
    }
  }

  else if (btype == B_USETENV) {
    if (c->argLength != 2 || c->redin != NULL
      || c->redout != NULL) {
      errorPrint("unsetenv takes one parameter", FPRINTF);
      return EXEC_FAIL;
    }
    if(unsetenv(c->arguments[1]) < 0) {
      errorPrint (NULL, PERROR);
      return EXEC_FAIL;
    }
  }

  else if (btype == B_CD) {
    if ((c->argLength != 1 && c->argLength != 2) 
      || c->redin != NULL || c->redout != NULL) {
      errorPrint("cd takes one parameter", FPRINTF);
      return EXEC_FAIL;
    }
    if (c->arguments[1] == NULL) c->arguments[1] = getenv("HOME"); 
    if (chdir(c->arguments[1]) < 0) {
      errorPrint(NULL, PERROR);
      return EXEC_FAIL;
    }
  }

  else if (btype == B_EXIT) {
    if (c->argLength != 1 || c->redin != NULL
      || c->redout != NULL) {
      errorPrint("exit does not take any parameters", FPRINTF);
      return EXEC_FAIL;
    }
    exit(0);
  }

  else {  // btype == B_FG or B_ALIAS
    errorPrint("fg and alias are not implemented", FPRINTF);
    return EXEC_FAIL;
  }

  return EXEC_SUCC;
}

/*--------------------------------------------------------------------*/

/* loads arguments array from the pending command at the back of the 
    pipedArg array of c. */
void loadPipetoArg (struct Command *c) {
  assert (c != NULL);

  int r, l = (c->pipeLength) - 1;
  r = l;
  while (c->pipedArgs[l] != NULL) {
    l--;
    if (l < 0) break;
  }
  for (int k = 0; k < r-l; k++) {
    c->arguments[k] = c->pipedArgs[k+l+1];
    c->pipedArgs[(k+l)+1] = NULL; //NULL loaded out entries
  }
  if ((l+1) != 0) l--;
  c->argLength = r-l;
  c->pipeLength = l + 1;
}

/*--------------------------------------------------------------------*/

/* executes command with arguments from arguments array and pipedArgs
    array in c. When error occurs, prints out appropriate messages to
    stderr and returns EXEC_FAIL . If no error, returns EXEC_SUCC. */
int execCommand (struct Command *c) {
  pid_t pid;
  int fd[2];
  int status;

  fflush(NULL);
  if ((pid = fork()) == 0) {
    if (input_fp != stdin) fclose(input_fp);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGALRM, SIG_IGN);

    // redirecting standard input if file name is given
    if (c->redin != NULL) {
      FILE *fp_i;
      if ((fp_i = fopen(c->redin, "r")) == NULL) {
        errorPrint(NULL, PERROR);
        exit(EXIT_FAILURE);
      }
      dup2(fileno(fp_i), STDIN_FILENO);
    }

    // redirecting standard input if file name is given
    if (c->redout != NULL) {
      FILE *fp_o;
        if ((fp_o = fopen(c->redout, "w")) == NULL) {
        errorPrint(NULL, PERROR);
        exit(EXIT_FAILURE);
      }
      chmod(c->redout, 0600);
      dup2(fileno(fp_o), STDOUT_FILENO);
    }
    
    // enters loop if given command is piped
    while (c->pipedArgs[0] != NULL) {
      memset(c->arguments, 0, (c->maxLength)*sizeof(char *));
      loadPipetoArg(c);

      pipe(fd);
      if (c->pipedArgs[0] != NULL) {
        fflush(NULL);
        if ((pid = fork()) == 0) {
          close(fd[0]);
          dup2(fd[1], STDOUT_FILENO);
          continue;
        }
        else {
          close(fd[1]);
          dup2(fd[0], STDIN_FILENO);
          waitpid(pid, NULL, 0);
          break;
        }
      }
    }

    if (execvp(c->arguments[0], c->arguments) < 0) {
      errorPrint(c->arguments[0], PERROR);
      exit(EXIT_FAILURE);
      }
  }
  else {
    waitpid(pid, &status, 0);
  }
  if (WEXITSTATUS(status) == EXIT_FAILURE) return EXEC_FAIL;
  return EXEC_SUCC;
}