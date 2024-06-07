/*  Name: Wang Jonghyuk
    Student ID: 20220425
    Description: This source code implements simple grep that searches
                 for the given pattern in stdin, then prints out each
                 line that contains the pattern to stdout.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for skeleton code */
#include <unistd.h> /* for getopt */
#include "str.h"

#define MAX_STR_LEN 1023

#define FALSE 0
#define TRUE  1

/*
 * Fill out your own functions here (If you need)
 */

/* SearchPatternLine()
   searches for substring pattern in string line. pattern can be simple
   string or it can have one or more '*' in the string. '*' matches 0 
   or more characters of any value except for newline. returns TRUE if
   pattern is found. if it is not found, returns FALSE.              */
int SearchPatternLine(const char* line, const char* pattern)
{
  char* starpoint;
  starpoint = StrFindChr(pattern, '*');
  if (starpoint != NULL) {
    *starpoint = '\0';
    line = StrFindStr(line, pattern);
    if (line == NULL) return FALSE;
    pattern = starpoint+1;
    if (SearchPatternLine(line, pattern) == FALSE) return FALSE;
  }
  else {
    line = StrFindStr(line, pattern);
    if (line == NULL) return FALSE;
  }
  return TRUE;
}

/*--------------------------------------------------------------------*/
/* PrintUsage()
   print out the usage of the Simple Grep Program                     */
/*--------------------------------------------------------------------*/
void
PrintUsage(const char* argv0)
{
  const static char *fmt =
	  "Simple Grep (sgrep) Usage:\n"
	  "%s pattern [stdin]\n";

  printf(fmt, argv0);
}
/*-------------------------------------------------------------------*/
/* SearchPattern()
   Your task:
   1. Do argument validation
   - String or file argument length is no more than 1023
   - If you encounter a command-line argument that's too long,
   print out "Error: pattern is too long"

   2. Read the each line from standard input (stdin)
   - If you encounter a line larger than 1023 bytes,
   print out "Error: input line is too long"
   - Error message should be printed out to standard error (stderr)

   3. Check & print out the line contains a given string (search-string)

   Tips:
   - fgets() is an useful function to read characters from file. Note
   that the fget() reads until newline or the end-of-file is reached.
   - fprintf(sderr, ...) should be useful for printing out error
   message to standard error

   NOTE: If there is any problem, return FALSE; if not, return TRUE  */
/*-------------------------------------------------------------------*/

/* SearchPattern()
   searches for string pattern in each line from stdin, then prints it
   to stdout if the pattern is found. gives error if string pattern or
   line from stdin is too long.                                     */
int
SearchPattern(const char *pattern)
{
  char buf[MAX_STR_LEN + 2];
  char pattern_copy[MAX_STR_LEN+1];

  /*
   *  TODO: check if pattern is too long
   */
  //printf("%ld\n", StrGetLength(pattern));
  if (StrGetLength(pattern) > MAX_STR_LEN) {
    fprintf(stderr, "Error: pattern is too long\n");
    return FALSE;
  }


  /* Read one line at a time from stdin, and process each line */
  while (fgets(buf, sizeof(buf), stdin)) {

    /* TODO: check the length of an input line */
    if (StrGetLength(buf) > MAX_STR_LEN) {
      fprintf(stderr, "Error: input line is too long\n");
      return FALSE;
    }

    /* TODO: fill out this function */
    StrCopy(pattern_copy, pattern);
    if (SearchPatternLine(buf, pattern_copy) == TRUE) printf("%s", buf);
  }

  return TRUE;
}
/*-------------------------------------------------------------------*/
int
main(const int argc, const char *argv[])
{
  /* Do argument check and parsing */
  if (argc < 2) {
	  fprintf(stderr, "Error: argument parsing error\n");
	  PrintUsage(argv[0]);
	  return (EXIT_FAILURE);
  }

  return SearchPattern(argv[1]) ? EXIT_SUCCESS:EXIT_FAILURE;
}
