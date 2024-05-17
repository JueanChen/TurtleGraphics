#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include "neillsimplescreen.h"

#define MAXNUMTOKENS 1000
#define MAXTOKENSIZE 20
#define MAXVARNUM 26
#define SETNUM 2
#define HEIGHT 33
#define WIDTH 51
#define ROWSTART 16
#define COLSTART 25
// Adjust the coordinates to make the picture to 
// be in the middle of the page in .ps file and .pdf file 
#define ROWADJUST 54 
#define COLADJUST 5
#define ANGLE1 0
#define ANGLE2 90
#define ANGLE3 180
#define ANGLE4 270
#define ANGLE5 360
#define DELAY 0.1
// Change the angle from degrees to radians 
// by multiplying RAD
#define RAD 0.01745329251 
#define strsame(A, B) (strcmp(A, B) == 0)
#define ERROR(PHRASE) { fprintf(stderr, \
   "Fatal Error %s occurred in %s, line %d\n", PHRASE, \
   __FILE__, __LINE__); \
   exit(EXIT_FAILURE); }

typedef struct prog {
   // wds store the whole input file
   char wds[MAXNUMTOKENS][MAXTOKENSIZE];

   // cw store the current word
   int cw;
} Program;

typedef struct output {
   // 51x33 output matrix for the .txt file
   char op[HEIGHT][WIDTH];

   // Current colour of FWD instruction
   char colour[MAXTOKENSIZE];

   // Current distance of FWD instruction
   char forward[MAXTOKENSIZE];

   // Current degree which will be added to 
   // the angle of FWD instruction
   char degree[MAXTOKENSIZE];

   // Current coordinates, angle of the FWD instruction,
   // whether it is needed to print out the picture to the screen,
   // the pointer of where to store the next set of coordinates and color
   // in r[MAXNUMTOKENS], c[MAXNUMTOKENS], color[MAXNUMTOKENS][MAXTOKENSIZE];  
   int row, col, angle, printout, pointer;

   // All the accurate coordinates and the color 
   // of each FWD instruction for the .ps file
   double r[MAXNUMTOKENS];
   double c[MAXNUMTOKENS];
   char color[MAXNUMTOKENS][MAXTOKENSIZE];
} Output;

typedef struct variables {
   // Stores all the variables of each VAR
   // Seperate space for each letter (e.g. A -> [0][][], B -> [1][][])
   char wds[MAXVARNUM][MAXNUMTOKENS][MAXTOKENSIZE];

   // Stack of the current VAR that is using
   char cv[MAXVARNUM];

   // Pointer of which variable of the VAR is currently using 
   int cw[MAXVARNUM];

   // Number of variables in each VAR
   int npv[MAXVARNUM];

   // Which loop the program is currently in, 
   // it is also the index of tpl[]
   int loopNum;

   // Number of tokens in each loop 
   int tpl[MAXVARNUM];

   // Stack of storing variables in SET instruction
   char setStack[MAXNUMTOKENS][MAXTOKENSIZE];

   // Pointer of the setStack (the next address to store)
   int setIndex;
} Variables;

// Start the program
// <PROG> ::= "START" <INSLST>
void Prog(Program *p, Output *o, Variables *v);

// Read the next word
void Next(Program *p);

// <INSLST> ::= "END" | <INS> <INSLST>
void Inslst(Program *p, Output *o, Variables *v);

// <INS> ::= <FWD> | <RGT> | <COL> | <LOOP> | <SET>
void Ins(Program *p, Output *o, Variables *v);

// Move forward instruction
// <FWD> ::= "FORWARD" <VARNUM>
void Fwd(Program *p, Output *o, Variables *v);

// Change the direction of the forward instruction clockwise
// <RGT> ::= "RIGHT" <VARNUM>
void Rgt(Program *p, Output *o, Variables *v);

// Change the colour of the forward instruction
// <COL> ::= "COLOUR" <VAR> | "COLOUR" <WORD>
void Col(Program *p, Output *o, Variables *v);

// Create a loop
// <LOOP> ::= "LOOP" <LTR> "OVER" <LST> <INSLST>
void Loop(Program *p, Output *o, Variables *v);

// Set the value of a variable
// <SET> ::= "SET" <LTR> "(" <PFIX>
void Set(Program *p, Output *o, Variables *v);

// Read a variable or a number
// <VARNUM> ::= <VAR> | <NUM>
void Varnum(Program *p, Output *o, Variables *v, char flag);

// Read a variable % Variables e.g. $A, $B, $Z etc.
// <VAR> ::= $<LTR>
void Var(Program *p, Variables *v, char *addr);

// Read a uppercase letter
// <LTR> ::= A, B ... Z
void Ltr(Program *p);

// Read a number (any valid double)
// <NUM> ::= 10 or -17.99 etc.
void Num(Program *p, char *addr);

// Read a single word (as defined by scanf("%s"...) with double-quotes around it
// Valid colours include "BLACK", "RED", "GREEN", "BLUE",
// "YELLOW", "CYAN", "MAGENTA", "WHITE"
// <WORD> ::= "RED", "BLUE", "HELLO!" or "178"
void Word(Program *p, char *addr);

// Read the list in LOOP
// <LST> ::= "{" <ITEMS>
void Lst(Program *p, Output *o, Variables *v);

// <ITEMS> ::= "}" | <ITEM> <ITEMS>
void Items(Program *p, Output *o, Variables *v);

// <ITEM> ::= <VARNUM> | <WORD>
void Item(Program *p, Output *o, Variables *v);

// <PFIX> ::= ")" | <OP> <PFIX> | <VARNUM> <PFIX>
// OP: A single mathematical operation character
// <OP> ::= + - / *
void Pfix(Program *p, Output *o, Variables *v, char *addr, int n);

// Initilize the output and variables
void Init(Output *o, Variables *v);

// Check if it is needed print out the picture to the screen
void is_printout(Output *o, int argc);

// Change a string of the colour to a letter (e.g. "RED" -> 'R')
char colour_short(char *str);

// Get the address of where to write the variable / number 
char *get_addr(Output *o, Variables *v, char flag);

// Calculate when meet operator in SET instruction
void calculate(Variables *v, char c);

// Write the colour according to the FWD 
// instruction to the 51x33 output matrix 
void print(Output *o);

// Record the accurate coordinates of the FWD instruction
void record(Output *o, double distance);

// Subfunction of print()
void subprint(Output *o);

// Subfunction of print()
void move(Output *o);

// Check whethe the coordinate is safe to write in the 51x33 matrix
bool is_safe(int row, int col);

// Using bresenham line algorithm to determine the coordinate
// to write according to the FWD instruction
void bresenham_line(Output *o, int x1, int y1, double distance);

// Subfunction of bresenham_line()
void plot_line_low(Output *o, int x1, int y1, int x2, int y2);

// Subfunction of bresenham_line()
void plot_line_high(Output *o, int x1, int y1, int x2, int y2);

// Print out the picture to the screen
void print_out(Output *o);

// Subfunction of print_out()
neillcol get_col(Output *o, int row, int col);

// Output the picture to the specified file
void write_file(Output *o, char *filename, FILE *fp);

// Subfunction of write_file()
void get_color(Output *o, int i, char *color);
