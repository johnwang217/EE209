/*  Name: Wang Jonghyuk
    Student ID: 20220425
    Description: This source code implements basic string library
                 functions. 
*/

#include <assert.h> /* to use assert() */
#include <stdio.h>
#include <stdlib.h> /* for strtol() */
#include <string.h>
#include <strings.h>
#include "str.h"
#include <ctype.h> /* for isspace(), isdigit(), isalpha() */

/* Your task is: 
   1. Rewrite the body of "Part 1" functions - remove the current
      body that simply calls the corresponding C standard library
      function.
   2. Write appropriate comment per each function
*/

/* Part 1 */


/* StrGetLength()
   calculates the length of string pcSrc while excluding the terminating
   null byte '\0'. returns value of length                            */
size_t StrGetLength(const char* pcSrc)
{
  const char *pcEnd;
  assert(pcSrc); /* NULL address, 0, and FALSE are identical. */
  pcEnd = pcSrc;
	
  while (*pcEnd) /* null character and FALSE are identical. */
    pcEnd++;

  return (size_t)(pcEnd - pcSrc);
}

/* StrCopy()
   copies string pointed to by pcSrc and stores it at the buffer pointed
   to by pcDest. returns pointer to string pcDest                     */
char *StrCopy(char *pcDest, const char* pcSrc)
{
  /* TODO: fill this function */
  size_t i = 0;
  assert(pcSrc != 0);
  
  while (*(pcSrc+i) != '\0') {
    *(pcDest+i) = *(pcSrc+i);
    i++;
  }
  *(pcDest+i) = '\0';

  return pcDest;
}

/* StrCompare()
   compares two strings pcS1 and pcS2. returns 0 when two strings are
   the same. when the strings do not match, returns the differnce of 
   ASCII value of two characters where the strings differentiate.  */
int StrCompare(const char* pcS1, const char* pcS2)
{
  /* TODO: fill this function */
  int cmp;
  assert((pcS1 != 0) && (pcS2 != 0));

  while (*pcS1 != '\0') {
    if (*pcS1 == *pcS2) {pcS1++; pcS2++;}
    else break;
  }

  if ((*pcS1 == '\0') && (*pcS2 == '\0')) cmp = 0;
  else cmp = (int)(*pcS1-*pcS2);

  return cmp;
}

/* StrFindChr()
   searches for character c in string pcHaystack. returns pointer to
   the first occurence of the character in the string. if the 
   character is not found, returns NULL                           */
char *StrFindChr(const char* pcHaystack, int c)
{
  /* TODO: fill this function */
  assert(pcHaystack != 0);

  while(*pcHaystack != c) {
    if (*pcHaystack == '\0') {pcHaystack = NULL; break;}
    pcHaystack++;
  }

  return (char*)pcHaystack;
}

/* StrFindStr()
   searches for substring pcNeedle in string pcHaystack. terminating
   null byte '\0' is not compared. returns pointer to the first 
   character of the substring in string. if the substring is not 
   found, returns NULL                                            */
char *StrFindStr(const char* pcHaystack, const char *pcNeedle)
{
  /* TODO: fill this function */
  #define FIND_CHR 0
  #define COMPARE_STR 1

  assert((pcHaystack != 0) && (pcNeedle != 0));

  size_t i = 0, state = FIND_CHR;
  while(*(pcNeedle+i) != '\0') {
    switch (state) {
      case FIND_CHR:
        pcHaystack = StrFindChr(pcHaystack, *pcNeedle);
        if (pcHaystack != NULL) {i++; state = COMPARE_STR;}
        else return NULL;
        break;
      
      case COMPARE_STR:
        if (*(pcHaystack+i) == *(pcNeedle+i)) i++;
        else {i=0; pcHaystack++; state = FIND_CHR;}
        break;
    
      default:
        assert(0); //Error: Should never get here
        break;
    }
  }
  
  return (char*)pcHaystack;
}

/* StrConcat()
   appends string pcSrc to string pcDest, overwriting the terminating
   null byte '\0' at the end of pcDest. returns a pointer to string 
   pcDest.                                                         */
char *StrConcat(char *pcDest, const char* pcSrc)
{
  /* TODO: fill this function */
  assert((pcDest != 0) && (pcSrc != 0));

  char* destend = StrFindChr(pcDest, '\0');
  if (destend == NULL) assert(0); //Error: pcDest isn't string

  while (*pcSrc != '\0') {
    *destend = *pcSrc;
    destend++;
    pcSrc++;
  }
  *destend = '\0';
  
  return pcDest;
}

/* check_ouflow()
   checks if overflow or underflow will occur when when the next
   calculation (-1)^(isminus) * q*10 + r is finished. returns 1 when 
   overflow or underflow will occur. if neither occurs, returns 0 */
int check_ouflow(long int q, int r, int isminus)
{
  if (isminus) {
    if (q>LONG_MAX/10) return 1;
    else if ((q == LONG_MAX/10) && ((r - '0') > 8)) return 1;
    else return 0; 
  }

  else {
    if (q>LONG_MAX/10) return 1;
    else if ((q == LONG_MAX/10) && ((r - '0') > 7)) return 1;
    else return 0;
  }
}

/* StrToLong()
   converts the initial part of string nptr to a decimal long integer.
   the string may begin with arbitrary amount of white space, followed
   by an optional '+' or '-' sign. then converts the following string 
   to a decimal long integer value, stopping when it reads a nondigit
   character. when endptr is not NULL, the function stores a pointer 
   to the first nondigit character at *endptr. returns the long integer
   value if overflow or underflow doesn't occur. if it does occur,
   function returns LONG_MIN when there is underflow and LONG_MAX when
   there is overflow.                                               */
long int StrToLong(const char *nptr, char **endptr, int base)
{
  assert(nptr != NULL);
  long int res = 0;
  int isminus = 0; // default sign is '+'

  /* handle only when base is 10 */
  if (base != 10) return 0;

  /* TODO: fill this function */
  while(*nptr && isspace(*nptr)) {
    nptr++;
  }

  if (*nptr == '+') {nptr++;}
  else if (*nptr == '-') {isminus = 1; nptr++;}

  while(*nptr && isdigit(*nptr)) {
    if (check_ouflow(res, *nptr, isminus)) {
      if (isminus) return LONG_MIN;
      else return LONG_MAX;
    }
    res = res*10 + (*nptr-'0');
    nptr++;
  }
  if (isminus) res = -res;

  if (endptr != NULL) *endptr = (char*)nptr;

  return res;
}

/* StrCaseCompare()
   compares two strings pcS1 and pcS2, while ignoring the case of the
   alphabet. returns 0 when two strings are the same. when the strings
   do not match, returns the differnce of ASCII value of two characters
   where the strings differentiate.                                  */
int StrCaseCompare(const char *pcS1, const char *pcS2)
{
  /* TODO: fill this function */
  int cmp;
  assert((pcS1 != 0) && (pcS2 != 0));

  while (*pcS1 != '\0') {
    if (*pcS1 == *pcS2) {pcS1++; pcS2++;}
    else if (isalpha(*pcS1) && isalpha(*pcS2) && abs(*pcS1-*pcS2)==32)
      {pcS1++; pcS2++;}
    else break;
  }

  if ((*pcS1 == '\0') && (*pcS2 == '\0')) cmp = 0;
  else cmp = (int)(tolower(*pcS1)-tolower(*pcS2));
  return cmp;
}