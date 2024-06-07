/* 
Name: Wang Jonghyuk
Student ID : 20220425
Description: This code implements a C program wc209, which prints the
             number of words, newlines and characters in the input text 
             while skipping comments 
*/

#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

enum DFAState {IN, OUT, SLASH, STAR, LINE_COMM, COMM};

//Global variable declaration
enum DFAState state = OUT, prev_state;
int word_count = 0, newline_count = 0, char_count = 0, error_line = 0;

//Function declaration
void change_state(enum DFAState s); 
void when_slash(int c);
void when_in(int c);
void when_out(int c);

/* reads characters from the standard input, 
    counts words, newlines and characters while skipping comments,
    then writes the count to the standard output;
    uses global variable word_count, newline_count, char_count, error_line, state */
int main(void) 
{
    int c;
    
    while ((c = getchar()) != EOF) {    
        switch (state) {
            case IN:
                when_in(c);
                break;

            case OUT:
                when_out(c);
                break;

            case SLASH:
                when_slash(c);
                break;

            case STAR:
                if (c == '/') {char_count++; change_state(OUT);}
                else {
                    if (c == '\n') {char_count++; newline_count++;}
                    change_state(COMM);
                }
                break;

            case LINE_COMM:
                if (c == '\n') {char_count++; newline_count++; change_state(OUT);}
                break;

            case COMM:
                if (c == '*') change_state(STAR);
                else if (c == '\n') {char_count++; newline_count++;}
                break;

            default:
                assert(0);  /* error */ 
                break;
        }
    }

    /* prints error when comment isn't terminated */
    if (state == COMM || state == STAR) fprintf(stderr, "Error: line %d: unterminated comment\n", error_line);

    else printf("%d %d %d\n", newline_count, word_count, char_count); 
    return EXIT_SUCCESS;
}

/* changes state to input state s while saving the current state in prev_state;
    uses global variable state, prev_state */
void change_state(enum DFAState s) 
{
    prev_state = state;
    state = s;
}

/* changes state and updates counts of word, newlines and characters when state is SLASH, depending on input int c;
    uses global variable word_count, newline_count, char_count, error_line, state, prev_state */
void when_slash(int c) {
    if (!isspace(c)) {
        if (c == '/') change_state(LINE_COMM);
        else if (c == '*') {error_line = newline_count+1; change_state(COMM);}
        else {
            if (prev_state == OUT) word_count++;
            char_count += 2;
            change_state(IN);
        }
    }
    else {
        if (prev_state == OUT) word_count++;
        char_count += 2;
        if (c == '\n') newline_count++;
        change_state(OUT);
    }
}

/* changes state and updates counts of word, newlines and characters when state is IN, depending on input int c;
    uses global variable newline_count, char_count, state */
void when_in(int c) {
    if (!isspace(c)) {
        if (c == '/') change_state(SLASH);
        else char_count++;
       }
    else {
        char_count++;
        if (c == '\n') newline_count++;
        change_state(OUT);
    }
}

/* changes state and updates counts of word, newlines and characters when state is OUT, depending on input int c;
    uses global variable word_count, newline_count, char_count, state */
void when_out(int c) {
    if (!isspace (c)) { 
        if (c == '/') change_state(SLASH);
        else {word_count++; char_count++; change_state(IN);}
    }
    else {
        char_count++;
        if (c == '\n') newline_count++;
    }
}