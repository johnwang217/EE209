#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "lexsyn.h"
#include "util.h"
#include "token.h"

/*--------------------------------------------------------------------*/
/* ish.c                                                              */
/* Original Author: Bob Dondero                                       */
/* Modified by : Park Ilwoo                                           */
/* Illustrate lexical analysis using a deterministic finite state     */
/* automaton (DFA)                                                    */
/*--------------------------------------------------------------------*/

void exitTimer(int signum) {
  alarm(0);
  _exit(0);
}

void SIGQUIT_Handler(int signum) {
  signal(SIGQUIT, exitTimer);
  const char *msg = "Type Ctrl-\\ again within 5 seconds to exit";
  write(STDOUT_FILENO, msg, strlen(msg));
  alarm(5);
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

  oTokens = DynArray_new(0);
  if (oTokens == NULL) {
    errorPrint("Cannot allocate memory", FPRINTF);
    exit(EXIT_FAILURE);
  }


  lexcheck = lexLine(inLine, oTokens);
  switch (lexcheck) {
    case LEX_SUCCESS:
      if (DynArray_getLength(oTokens) == 0)
        return;

      /* dump lex result when DEBUG is set */
      dumpLex(oTokens);

      syncheck = syntaxCheck(oTokens);
      if (syncheck == SYN_SUCCESS) {
        /* TODO */
        char **arguments = malloc((oTokens->iLength + 1)*sizeof(char *));
        int i = 0;
        for (; i < oTokens->iLength; i++) {
          arguments[i] = ((struct Token *)oTokens->ppvArray[i])->pcValue;
        }
        arguments[i] = NULL;

        btype = checkBuiltin(DynArray_get(oTokens, 0));
        if (btype == B_SETENV) {
          if (arguments[2] == NULL) arguments[2] = "";
          setenv(arguments[1], arguments[2], 1);
        }
        else if (btype == B_USETENV) {
          unsetenv(arguments[1]);
        }
        else if (btype == B_CD) {
          chdir(arguments[1]);
        }
        else if (btype == B_EXIT) {
          exit(0);
        }
        else if (btype == NORMAL) {
          fflush(NULL);
          if ((pid = fork()) == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGALRM, SIG_IGN);

            if (execvp(arguments[0], arguments) < 0) {
              fprintf(stderr, "failed to execute command: %s\n", strerror(errno));
              exit(0);
              }
          }
          else {
            waitpid(pid, NULL, 0);
            free(arguments);
          }
        }
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
      break;

    case LEX_QERROR:
      errorPrint("Unmatched quote", FPRINTF);
      break;

    case LEX_NOMEM:
      errorPrint("Cannot allocate memory", FPRINTF);
      break;

    case LEX_LONG:
      errorPrint("Command is too large", FPRINTF);
      break;

    default:
      errorPrint("lexLine needs to be fixed", FPRINTF);
      exit(EXIT_FAILURE);
  }
}

int main() {
  /* TODO */
  sigset_t sSet;

  sigemptyset(&sSet);
  sigaddset(&sSet, SIGALRM);
  sigaddset(&sSet, SIGINT);
  sigaddset(&sSet, SIGQUIT);
  sigprocmask(SIG_UNBLOCK, &sSet, NULL);
  
  signal(SIGINT, SIG_IGN);
  signal(SIGALRM, SIGALARM_Handler);
  signal(SIGQUIT, SIGQUIT_Handler);

  char acLine[MAX_LINE_SIZE + 2];
  FILE *fp;
  char *homeDir = strdup(getenv("HOME"));
  homeDir = realloc(homeDir, strlen(homeDir) + 8);
  strcat(homeDir, "/.ishrc");

  if ((fp = fopen(homeDir, "r")) == NULL) {
    fp = stdin;
  }
  free(homeDir);
  while (1) {
    fprintf(stdout, "%% ");
    fflush(stdout);
    if (fgets(acLine, MAX_LINE_SIZE, fp) == NULL) {
      printf("\n");
      exit(EXIT_SUCCESS);
    }
    if (fp != stdin) fprintf(stdout, "%s", acLine);
    shellHelper(acLine);
  }
}

