#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "ish.h"
#include "lexsyn.h"
#include "token.h"

/*--------------------------------------------------------------------*/
/* ish.c                                                              */
/* Original Author: Bob Dondero                                       */
/* Modified by : Park Ilwoo                                           */
/* Illustrate lexical analysis using a deterministic finite state     */
/* automaton (DFA)                                                    */
/*--------------------------------------------------------------------*/

void SIGQUIT_Handler(int signum) {
  signal(SIGQUIT, exitTimer);
  const char *msg = "\nType Ctrl-\\ again within 5 seconds to exit\n";
  write(STDOUT_FILENO, msg, strlen(msg));
  alarm(5);
}

void exitTimer(int signum) {
  alarm(0);
  _exit(0);
}

void SIGALARM_Handler(int signum) {
  signal(SIGQUIT, SIGQUIT_Handler);
}

static void
shellHelper(const char *inLine) {
  DynArray_T oTokens;

  enum LexResult lexcheck;
  enum SyntaxResult syncheck;
  enum BuiltinType btype;

  pid_t pid;
  int fd[2];

  oTokens = DynArray_new(0);
  if (oTokens == NULL) {
    errorPrint("Cannot allocate memory", FPRINTF);
    exit(EXIT_FAILURE);
  }

  lexcheck = lexLine(inLine, oTokens);
  switch (lexcheck) {
    case LEX_SUCCESS:
      if (DynArray_getLength(oTokens) == 0) {
        DynArray_free(oTokens);
        return;
      }

      /* dump lex result when DEBUG is set */
      dumpLex(oTokens);

      syncheck = syntaxCheck(oTokens);
      if (syncheck == SYN_SUCCESS) {
        /* TODO */
        struct Command *input = buildCommand(oTokens);
        if (input == NULL) {
          errorPrint("Failed to allocate memory", FPRINTF);
          exit(EXIT_FAILURE);
        }

        btype = checkBuiltin(DynArray_get(oTokens, 0));

        if (btype != NORMAL) {
          if (execBuiltin(btype, input) < 0) {
            freeCommand(input);
            freeArrayTokens(oTokens);
            DynArray_free(oTokens);
            break;
          }
        }
        
        else {
          fflush(NULL);
          if ((pid = fork()) == 0) {
            
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGALRM, SIG_IGN);

            if (input->redin != NULL) {
              FILE *fp_i;
              if ((fp_i = fopen(input->redin, "r")) == NULL) {
                errorPrint(NULL, PERROR);
                exit(1);
              }
              dup2(fileno(fp_i), STDIN_FILENO);
            }

            if (input->redout != NULL) {
              FILE *fp_o;
                if ((fp_o = fopen(input->redout, "w")) == NULL) {
                errorPrint(NULL, PERROR);
                exit(1);
              }
              chmod(input->redout, 0600);
              dup2(fileno(fp_o), STDOUT_FILENO);
            }
            
            int r, l = (input->pip_index) - 1;

            while (input->pipes[0] != NULL) {
              memset(input->arguments, 0, (DynArray_getLength(oTokens) + 1)*sizeof(char *));
              pipe(fd);
              //load end of pipe to arg
              r = l;
              while (input->pipes[l] != NULL) {
                l--;
                if (l < 0) break;
              }
              for (int k = 0; k < r-l; k++) {
                input->arguments[k] = input->pipes[k+l+1];
                input->pipes[(k+l)+1] = NULL; //NULL loaded out entries
              }
              l--;

              if (input->pipes[0] != NULL) {
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

            if (execvp(input->arguments[0], input->arguments) < 0) {
              errorPrint(input->arguments[0], PERROR);
              exit(0);
              }
          }
          else {
            waitpid(pid, NULL, 0);
          }
        }
        freeCommand(input);
      }

      /* syntax error cases */
      else if (syncheck == SYN_FAIL_NOCMD)
        errorPrint("Missing command name", FPRINTF);
      else if (syncheck == SYN_FAIL_MULTREDOUT)
        errorPrint("Multiple redirection of standard out", FPRINTF);
      else if (syncheck == SYN_FAIL_NODESTOUT)
        errorPrint("Standard output redirection without file name", FPRINTF);
      else if (syncheck == SYN_FAIL_MULTREDIN)
        errorPrint("Multiple redirection of standard input", FPRINTF);
      else if (syncheck == SYN_FAIL_NODESTIN)
        errorPrint("Standard input redirection without file name", FPRINTF);
      else if (syncheck == SYN_FAIL_INVALIDBG)
        errorPrint("Invalid use of background", FPRINTF);
      
      freeArrayTokens(oTokens);
      DynArray_free(oTokens);
      break;

    case LEX_QERROR:
      errorPrint("Unmatched quote", FPRINTF);
      freeArrayTokens(oTokens);
      DynArray_free(oTokens);
      break;

    case LEX_NOMEM:
      errorPrint("Cannot allocate memory", FPRINTF);
      freeArrayTokens(oTokens);
      DynArray_free(oTokens);
      break;

    case LEX_LONG:
      errorPrint("Command is too large", FPRINTF);
      freeArrayTokens(oTokens);
      DynArray_free(oTokens);
      break;

    default:
      errorPrint("lexLine needs to be fixed", FPRINTF);
      exit(EXIT_FAILURE);
  }
}

int execBuiltin (enum BuiltinType btype, struct Command *c) {

  if (btype == B_SETENV) {
    if ((c->arg_index != 2 && c->arg_index != 3) 
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
    if (c->arg_index != 2 || c->redin != NULL
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
    if (c->arg_index != 2 || c->redin != NULL
      || c->redout != NULL) {
      errorPrint("cd takes one parameter", FPRINTF);
      return EXEC_FAIL;
    }
    if (chdir(c->arguments[1]) < 0) {
      errorPrint(NULL, PERROR);
      return EXEC_FAIL;
    }
  }

  else if (btype == B_EXIT) {
    if (c->arg_index != 1 || c->redin != NULL
      || c->redout != NULL) {
      errorPrint("exit does not take any parameters", FPRINTF);
      return EXEC_FAIL;
    }
    exit(0);
  }

  //btype == B_FG or B_ALIAS
  else { 
    errorPrint("fg and alias are not implemented", FPRINTF);
    return EXEC_FAIL;
  }

  return EXEC_SUCC;
}

int main(int argc, char* argv[]) {
  /* TODO */
  sigset_t sSet;

  /* to make sure these signals aren't blocked */
  sigemptyset(&sSet);
  sigaddset(&sSet, SIGALRM);
  sigaddset(&sSet, SIGINT);
  sigaddset(&sSet, SIGQUIT);
  sigprocmask(SIG_UNBLOCK, &sSet, NULL);
  
  signal(SIGINT, SIG_IGN);
  signal(SIGALRM, SIGALARM_Handler);
  signal(SIGQUIT, SIGQUIT_Handler);

  char acLine[MAX_LINE_SIZE + 2];

  char *homeDir = strdup(getenv("HOME"));
  homeDir = realloc(homeDir, strlen(homeDir) + 8);
  strcat(homeDir, "/.ishrc");

  FILE *fp;
  if ((fp = fopen(homeDir, "r")) == NULL) {
    fp = stdin;
  }
  free(homeDir);

  errorPrint(argv[0], SETUP);

  while (1) {
    fprintf(stdout, "%% ");
    fflush(stdout);
  skip:
    if (fgets(acLine, MAX_LINE_SIZE, fp) == NULL) {
      if (fp != stdin) {
        fp = stdin;
        goto skip;
      }
      else {
        printf("\n");
        exit(EXIT_SUCCESS);
      }
    }
    if (fp != stdin) {
      if (strchr(acLine, '\n') == NULL) {
        char* c = strchr(acLine, '\0');
        *c = '\n';
      }
      fprintf(stdout, "%s", acLine);
    }
    shellHelper(acLine);
  }
}

