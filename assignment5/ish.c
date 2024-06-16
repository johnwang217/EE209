/*--------------------------------------------------------------------*/
/* ish.c                                                              */
/* Author: Wang Jonghyuk                                              */
/* Student ID: 20220425                                               */
/* Description: implements interactive UNIX shell                     */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "lexsyn.h"
#include "util.h"
#include "exec.h"

/*--------------------------------------------------------------------*/

/* a SIGQUIT handler that terminates process */
void exitTimer(int signum) {
  alarm(0);
  _exit(0);
}

/* a SIGQUIT handler that sets off an alarm of 5 seconds to raise 
    another SIGQUIT to terminate process. */
void SIGQUIT_Handler(int signum) {
  signal(SIGQUIT, exitTimer);
  const char *msg = "\nType Ctrl-\\ again within 5 seconds to exit.\n";
  write(STDOUT_FILENO, msg, strlen(msg));
  alarm(5);
}

/* a SIGALARM handler that cancels pending termination. */
void SIGALARM_Handler(int signum) {
  signal(SIGQUIT, SIGQUIT_Handler);
}

/*--------------------------------------------------------------------*/

/* processes command recieved from inLine. If error occurs, prints
    appropriate messages to stderr. */
static void
shellHelper(const char *inLine) {
  DynArray_T oTokens;

  enum LexResult lexcheck;
  enum SyntaxResult syncheck;
  enum BuiltinType btype;

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
          errorPrint("Cannot allocate memory", FPRINTF);
          exit(EXIT_FAILURE);
        }

        btype = checkBuiltin(DynArray_get(oTokens, 0));

        if (btype != NORMAL) execBuiltin(input, btype);
        else execCommand(input);
        
        freeCommand(input);
      }

      /* syntax error cases */
      else if (syncheck == SYN_FAIL_NOCMD)
        errorPrint("Missing command name", FPRINTF);
      else if (syncheck == SYN_FAIL_MULTREDOUT)
        errorPrint("Multiple redirection of standard out", FPRINTF);
      else if (syncheck == SYN_FAIL_NODESTOUT)
        errorPrint("Standard output redirection without file name",
           FPRINTF);
      else if (syncheck == SYN_FAIL_MULTREDIN)
        errorPrint("Multiple redirection of standard input", FPRINTF);
      else if (syncheck == SYN_FAIL_NODESTIN)
        errorPrint("Standard input redirection without file name",
           FPRINTF);
      else if (syncheck == SYN_FAIL_INVALIDBG)
        errorPrint("Invalid use of background", FPRINTF);
      
      freeArrayTokens(oTokens);
      break;

    case LEX_QERROR:
      errorPrint("Unmatched quote", FPRINTF);
      freeArrayTokens(oTokens);
      break;

    case LEX_NOMEM:
      errorPrint("Cannot allocate memory", FPRINTF);
      freeArrayTokens(oTokens);
      break;

    case LEX_LONG:
      errorPrint("Command is too large", FPRINTF);
      freeArrayTokens(oTokens);
      break;

    default:
      errorPrint("lexLine needs to be fixed", FPRINTF);
      exit(EXIT_FAILURE);
  }
}

/*--------------------------------------------------------------------*/

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

  /* opens .ishrc file from home directory if it exists and can be read.
      redirects it to standard input stream. */

  char *homeDir = strdup(getenv("HOME"));
  homeDir = realloc(homeDir, strlen(homeDir) + 8);
  strcat(homeDir, "/.ishrc");

  FILE *fp;
  if ((fp = fopen(homeDir, "r")) == NULL) {
    fp = stdin; // when .ishrc doesn't exist or is not readable
  }
  free(homeDir);

  errorPrint(argv[0], SETUP);

  while (1) {
    fprintf(stdout, "%% ");
    fflush(stdout);
  start:
    if (fgets(acLine, MAX_LINE_SIZE, fp) == NULL) {
      if (fp != stdin) { // if done reading from .ishrc file
        fp = stdin;      // redirect input back to standard input 
        goto start;
      }
      else {
        printf("\n");
        exit(EXIT_SUCCESS);
      }
    }
    if (fp != stdin) {
      // appends nextline if it doesn't exist
      if (strchr(acLine, '\n') == NULL) {
        char* c = strchr(acLine, '\0');
        *c = '\n';
      }
      fprintf(stdout, "%s", acLine);
    }
    shellHelper(acLine);
  }
}

