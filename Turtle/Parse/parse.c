#include "parse.h"

int main(int argc, char *argv[]) 
{
   FILE *fp;
   fp = fopen(argv[1], "r");
   if (fp == NULL || argc < 2) {
      printf("Error: file not found or no input file.\n");
      return 1;
   }
   Program *prog = calloc(1, sizeof(Program));
   int i = 0;

   while (fscanf(fp, "%s", prog->wds[i++])==1 && i < MAXNUMTOKENS);
   assert(i < MAXNUMTOKENS);
   Prog(prog);
   fclose(fp);
   free(prog);
   
   return 0;
}

// Start the program
// <PROG> ::= "START" <INSLST>
void Prog(Program *p) {
   if (!strsame(p->wds[p->cw], "START")) {
      ERROR("No START statement ?\n");
   }
   Next(p);
   Inslst(p);
}

// Read the next word
void Next(Program *p) {
   p->cw = p->cw + 1;
}

// <INSLST> ::= "END" | <INS> <INSLST>
void Inslst(Program *p) {
   if (strsame(p->wds[p->cw], "END")) {
      return;
   }
   Ins(p);
   Next(p);
   Inslst(p);
}

// <INS> ::= <FWD> | <RGT> | <COL> | <LOOP> | <SET>
void Ins(Program *p) {
   if (strsame(p->wds[p->cw], "FORWARD")) {
      Fwd(p);
      return;
   }
   if (strsame(p->wds[p->cw], "RIGHT")) {
      Rgt(p);
      return;
   }
   if (strsame(p->wds[p->cw], "COLOUR")) {
      Col(p);
      return;
   }
   if (strsame(p->wds[p->cw], "LOOP")) {
      Loop(p);
      return;
   }
   if (strsame(p->wds[p->cw], "SET")) {
      Set(p);
      return;
   }
   ERROR("Expecting a FORWARD/RIGHT/COLOUR/LOOP/SET ?");
}

// Move forward instruction
// <FWD> ::= "FORWARD" <VARNUM>
void Fwd(Program *p) {
   Next(p);
   Varnum(p);
   return;
}

// Change the direction of the forward instruction clockwise
// <RGT> ::= "RIGHT" <VARNUM>
void Rgt(Program *p) {
   Next(p);
   Varnum(p);
   return;
}

// Change the colour of the forward instruction
// <COL> ::= "COLOUR" <VAR> | "COLOUR" <WORD>
void Col(Program *p) {
   Next(p);
   if (p->wds[p->cw][0] != '$') {
      Word(p);
      return;
   }
   else {
      Var(p);
      return;
   }
}

// Create a loop
// <LOOP> ::= "LOOP" <LTR> "OVER" <LST> <INSLST>
void Loop(Program *p) {
   Next(p);
   Ltr(p);
   Next(p);
   if (!strsame(p->wds[p->cw], "OVER")) {
      ERROR("Expecting a OVER ?")
   }
   Next(p);
   Lst(p);
   Next(p);
   Inslst(p);
}

// Set the value of a variable
// <SET> ::= "SET" <LTR> "(" <PFIX>
void Set(Program *p) {
   Next(p);
   Ltr(p);
   Next(p);
   if (p->wds[p->cw][0] == '(') {
      Next(p);
      Pfix(p);
      return;
   }
   ERROR("Expecting a ( ?");
}

// Read a variable or a number
// <VARNUM> ::= <VAR> | <NUM>
void Varnum(Program *p) {
   if (isdigit(p->wds[p->cw][0]) || p->wds[p->cw][0] == '-') {
      Num(p);
      return;
   }
   else {
      Var(p);
      return;
   }
}

// Read a variable % Variables e.g. $A, $B, $Z etc.
// <VAR> ::= $<LTR>
void Var(Program *p) {
   if (p->wds[p->cw][0] == '$') {
      Ltr(p);
      return;
   }
   ERROR("Expecting a variable ($A, $B, $Z etc.) ?");
}

// Read a uppercase letter
// <LTR> ::= A, B ... Z
void Ltr(Program *p) {
   if (p->wds[p->cw][0] == '$') {
      if (!isupper(p->wds[p->cw][1]) || strlen(p->wds[p->cw]) > 2) {
         ERROR("Expecting one uppercase letter (A, B ... Z) ?");
      }
   }
   else {
      if (!isupper(p->wds[p->cw][0]) || strlen(p->wds[p->cw]) > 2) {
         ERROR("Expecting one uppercase letter (A, B ... Z) ?");
      }
   }

   return;
}

// Read a number (any valid double)
// <NUM> ::= 10 or -17.99 etc.
void Num(Program *p) {
   int point_num = 0; // number of the decial point

   for (int i = 1; i < (int)strlen(p->wds[p->cw]); i++) {
      if (p->wds[p->cw][i] == '.') {
         point_num++;
      }
      if (!isdigit(p->wds[p->cw][i]) && p->wds[p->cw][i] != '.') {
         ERROR("Expecting a number (10, -17.99 etc.) ?");
      }
   }
   if (point_num > 1) {
      ERROR("Expecting a number (10, -17.99 etc.) ?");
   }

   return;
}

// Read a single word (as defined by scanf("%s"...) with double-quotes around it
// Valid colours include "BLACK", "RED", "GREEN", "BLUE",
// "YELLOW", "CYAN", "MAGENTA", "WHITE"
// <WORD> ::= "RED", "BLUE", "HELLO!" or "178"
void Word(Program *p) {
   int i = 0;

   if (p->wds[p->cw][i] != '"') {
      ERROR("Expecting a single word with \
             double-quotes around it (\"RED\", \"178\" etc.) ?");
   }
   for (i = 1; i < (int)strlen(p->wds[p->cw])-1; i++) {
      if (!isdigit(p->wds[p->cw][i]) && !isupper(p->wds[p->cw][i])) {
         ERROR("Expecting a single word with \
                double-quotes around it (\"RED\", \"178\" etc.) ?");
      }
   }
   if (p->wds[p->cw][i] != '"' || i == 1) {
      ERROR("Expecting a single word with \
             double-quotes around it (\"RED\", \"178\" etc.) ?");
   }

   return;
}

// Read the list in LOOP
// <LST> ::= "{" <ITEMS>
void Lst(Program *p) {
   if (p->wds[p->cw][0] == '{') {
      Next(p);
      Items(p);
      return;
   }
   ERROR("Expecting a { ?");
}

// <ITEMS> ::= "}" | <ITEM> <ITEMS>
void Items(Program *p) {
   if (p->wds[p->cw][0] == '}') {
      return;
   }
   else {
      Item(p);
      Next(p);
      Items(p);
   }   
}

// <ITEM> ::= <VARNUM> | <WORD>
void Item(Program *p) {
   if (p->wds[p->cw][0] == '"') {
      Word(p);
      return;
   }
   else {
      Varnum(p);
      return;
   }   
}

// <PFIX> ::= ")" | <OP> <PFIX> | <VARNUM> <PFIX>
// OP: A single mathematical operation character
// <OP> ::= + - / *
void Pfix(Program *p) {
   char c = p->wds[p->cw][0];

   if (c == ')') {
      return;
   }
   else if (c == '+' || c == '-' || c == '/' || c == '*') {
      if ((int)strlen(p->wds[p->cw]) > 1) {
         ERROR("Expecting a single mathematical operation character (+ - / *)")
      }
      Next(p);
      Pfix(p);
      return;
   }
   else {
      Varnum(p);
      Next(p);
      Pfix(p);
      return;
   }
}
