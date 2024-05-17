#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAXNUMTOKENS 100
#define MAXTOKENSIZE 20
#define strsame(A, B) (strcmp(A, B) == 0)
#define ERROR(PHRASE) { fprintf(stderr, \
   "Fatal Error %s occurred in %s, line %d\n", PHRASE, \
   __FILE__, __LINE__); \
   exit(EXIT_FAILURE); }

typedef struct  prog {
   // wds store the whole input file
   char wds[MAXNUMTOKENS][MAXTOKENSIZE];

   // cw store the current word
   int cw;
} Program;

// Start the program
// <PROG> ::= "START" <INSLST>
void Prog(Program *p);

// Read the next word
void Next(Program *p);

// <INSLST> ::= "END" | <INS> <INSLST>
void Inslst(Program *p);

// <INS> ::= <FWD> | <RGT> | <COL> | <LOOP> | <SET>
void Ins(Program *p);

// Move forward instruction
// <FWD> ::= "FORWARD" <VARNUM>
void Fwd(Program *p);

// Change the direction of the forward instruction clockwise
// <RGT> ::= "RIGHT" <VARNUM>
void Rgt(Program *p);

// Change the colour of the forward instruction
// <COL> ::= "COLOUR" <VAR> | "COLOUR" <WORD>
void Col(Program *p);

// Create a loop
// <LOOP> ::= "LOOP" <LTR> "OVER" <LST> <INSLST>
void Loop(Program *p);

// Set the value of a variable
// <SET> ::= "SET" <LTR> "(" <PFIX>
void Set(Program *p);

// Read a variable or a number
// <VARNUM> ::= <VAR> | <NUM>
void Varnum(Program *p);

// Read a variable % Variables e.g. $A, $B, $Z etc.
// <VAR> ::= $<LTR>
void Var(Program *p);

// Read a uppercase letter
// <LTR> ::= A, B ... Z
void Ltr(Program *p);

// Read a number (any valid double)
// <NUM> ::= 10 or -17.99 etc.
void Num(Program *p);

// Read a single word (as defined by scanf("%s"...) with double-quotes around it
// Valid colours include "BLACK", "RED", "GREEN", "BLUE",
// "YELLOW", "CYAN", "MAGENTA", "WHITE"
// <WORD> ::= "RED", "BLUE", "HELLO!" or "178"
void Word(Program *p);

// Read the list in LOOP
// <LST> ::= "{" <ITEMS>
void Lst(Program *p);

// <ITEMS> ::= "}" | <ITEM> <ITEMS>
void Items(Program *p);

// <ITEM> ::= <VARNUM> | <WORD>
void Item(Program *p);

// <PFIX> ::= ")" | <OP> <PFIX> | <VARNUM> <PFIX>
// OP: A single mathematical operation character
// <OP> ::= + - / *
void Pfix(Program *p);
