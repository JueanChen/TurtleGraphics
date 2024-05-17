#include "extension.h"

int main(void) 
{
   int inputChoice, outputChoice, i = 0;
   char fileName[MAXNUMTOKENS];
   FILE *fp1;

   inputChoice = get_input_choice();
   if (inputChoice == 1) {
      get_filename(fileName);
   }
   else {
      input_new_file(fileName);
   }
   fp1 = fopen(fileName, "r");
   if (fp1 == NULL) {
      printf("Error: file not found or no input file.\n");
      return 1;
   }
   Program *prog = calloc(1, sizeof(Program));
   Output *output = calloc(1, sizeof(Output));
   Variables *variables = calloc(1, sizeof(Variables));

   outputChoice = get_output_choice();
   is_printout(output, outputChoice);
   while (fscanf(fp1, "%s", prog->wds[i++])==1 && i < MAXNUMTOKENS);
   assert(i < MAXNUMTOKENS);
   Init(output, variables);
   Prog(prog, output, variables);
   
   if (outputChoice != 1) {
      FILE *fp2;
      change_filename(fileName, outputChoice);
      fp2 = fopen(fileName, "w");
      if (fp2 == NULL) {
         printf("Error: file not found.\n");
         return 1;
      }
      write_file(output, fileName, fp2);
   }
   fclose(fp1);
   free(prog);
   free(output);
   free(variables);
   printf("Program completed, have a good day :)\n");
   
   return 0;
}

// Start the program
// <PROG> ::= "START" <INSLST>
void Prog(Program *p, Output *o, Variables *v) {
   if (!strsame(p->wds[p->cw], "START")) {
      ERROR("No START statement ?\n");
   }
   Next(p);
   Inslst(p, o, v);
}

// Read the next word
void Next(Program *p) {
   p->cw = p->cw + 1;
}

// <INSLST> ::= "END" | <INS> <INSLST>
void Inslst(Program *p, Output *o, Variables *v) {
   if (strsame(p->wds[p->cw], "END")) {
      return;
   }
   Ins(p, o, v);
   Next(p);
   Inslst(p, o, v);
}

// <INS> ::= <FWD> | <RGT> | <COL> | <LOOP> | <SET>
void Ins(Program *p, Output *o, Variables *v) {
   if (strsame(p->wds[p->cw], "FORWARD")) {
      Fwd(p, o, v);
      return;
   }
   if (strsame(p->wds[p->cw], "RIGHT")) {
      Rgt(p, o, v);
      return;
   }
   if (strsame(p->wds[p->cw], "COLOUR")) {
      Col(p, o, v);
      return;
   }
   if (strsame(p->wds[p->cw], "LOOP")) {
      Loop(p, o, v);
      return;
   }
   if (strsame(p->wds[p->cw], "SET")) {
      Set(p, o, v);
      return;
   }
   ERROR("Expecting a FORWARD/RIGHT/COLOUR/LOOP/SET ?");
}

// Move forward instruction
// <FWD> ::= "FORWARD" <VARNUM>
void Fwd(Program *p, Output *o, Variables *v) {
   Next(p);
   Varnum(p, o, v, 'F');
   print(o);
   o->pointer++;
   return;
}

// Change the direction of the forward instruction clockwise
// <RGT> ::= "RIGHT" <VARNUM>
void Rgt(Program *p, Output *o, Variables *v) {
   Next(p);
   Varnum(p, o, v, 'R');
   int degree = atoi(o->degree);
   o->angle += degree;
   if (o->angle < 0) {
      o->angle += 360;
   }
   if (o->angle > 360) {
      o->angle -= 360;
   }
   return;
}

// Change the colour of the forward instruction
// <COL> ::= "COLOUR" <VAR> | "COLOUR" <WORD>
void Col(Program *p, Output *o, Variables *v) {
   Next(p);
   char *addr = get_addr(o, v, 'C');
   if (p->wds[p->cw][0] != '$') {
      Word(p, o->colour);
      return;
   }
   else {
      Var(p, v, addr);
      return;
   }
}

// Create a loop
// <LOOP> ::= "LOOP" <LTR> "OVER" <LST> <INSLST>
void Loop(Program *p, Output *o, Variables *v) {
   Next(p);
   Ltr(p);
   v->cv[v->loopNum] = p->wds[p->cw][0];
   Next(p);
   if (!strsame(p->wds[p->cw], "OVER")) {
      ERROR("Expecting a OVER ?")
   }
   Next(p);
   Lst(p, o, v);

   int pos = v->cv[v->loopNum] - 'A';
   for (int i = 0; i < v->npv[pos]; i++) {
      v->tpl[v->loopNum] = 0;
      v->cw[pos] = i;
      int cp = p->cw;
      int num_end = 1;
      while (num_end != 0) {
         if (strsame(p->wds[cp], "END")) {
            num_end--;
         }
         cp++;
         if (strsame(p->wds[cp], "LOOP")) {
            num_end++;
         }
         v->tpl[v->loopNum]++;
      }
      v->loopNum++;
      Next(p);
      Inslst(p, o, v);
      v->loopNum--;
      p->cw -= v->tpl[v->loopNum];
   }
   p->cw += v->tpl[v->loopNum];
   Inslst(p, o, v);
}

// Set the value of a variable
// <SET> ::= "SET" <LTR> "(" <PFIX>
void Set(Program *p, Output *o, Variables *v) {
   Next(p);
   Ltr(p);
   v->setIndex = 0;
   int pos = p->wds[p->cw][0] - 'A';
   char *addr = v->wds[pos][v->cw[pos]];
   Next(p);
   if (p->wds[p->cw][0] == '(') {
      Next(p);
      int n = 0;
      int cp = p->cw;
      while (p->wds[cp][0] != ')') {
         cp++;
         n++;
      }
      Pfix(p, o, v, addr, n);
      return;
   }
   ERROR("Expecting a ( ?");
}

// Read a variable or a number
// <VARNUM> ::= <VAR> | <NUM>
void Varnum(Program *p, Output *o, Variables *v, char flag) {
   char *addr = get_addr(o, v, flag);
   if (isdigit(p->wds[p->cw][0]) || p->wds[p->cw][0] == '-') {
      Num(p, addr);
      return;
   }
   else {
      Var(p, v, addr);
      return;
   }
}

// Read a variable % Variables e.g. $A, $B, $Z etc.
// <VAR> ::= $<LTR>
void Var(Program *p, Variables *v, char *addr) {
   if (p->wds[p->cw][0] == '$') {
      Ltr(p);
      char var = p->wds[p->cw][1];
      int pos = var - 'A';
      strcpy(addr, v->wds[pos][v->cw[pos]]);
      return;
   }
   ERROR("Expecting a variable ($A, $B, $Z etc.) ?");
}

// Read a uppercase letter
// <LTR> ::= A, B ... Z
void Ltr(Program *p) {
   if (p->wds[p->cw][0] == '$') {
      if (!isupper(p->wds[p->cw][1]) || strlen(p->wds[p->cw]) > SETNUM) {
         ERROR("Expecting one uppercase letter (A, B ... Z) ?");
      }
   }
   else {
      if (!isupper(p->wds[p->cw][0]) || strlen(p->wds[p->cw]) > SETNUM) {
         ERROR("Expecting one uppercase letter (A, B ... Z) ?");
      }
   }

   return;
}

// Read a number (any valid double)
// <NUM> ::= 10 or -17.99 etc.
void Num(Program *p, char *addr) {
   int point_num = 0;

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
   strcpy(addr, p->wds[p->cw]);

   return;
}

// Read a single word (as defined by scanf("%s"...) with double-quotes around it
// Valid colours include "BLACK", "RED", "GREEN", "BLUE",
// "YELLOW", "CYAN", "MAGENTA", "WHITE"
// <WORD> ::= "RED", "BLUE", "HELLO!" or "178"
void Word(Program *p, char *addr) {
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
   if (p->wds[p->cw][i] != '"') {
      ERROR("Expecting a single word with \
             double-quotes around it (\"RED\", \"178\" etc.) ?");
   }
   strcpy(addr, p->wds[p->cw]);

   return;
}

// Read the list in LOOP
// <LST> ::= "{" <ITEMS>
void Lst(Program *p, Output *o, Variables *v) {
   if (p->wds[p->cw][0] == '{') {
      Next(p);
      char c = v->cv[v->loopNum];
      int pos = c - 'A';
      v->cw[pos] = 0;
      Items(p, o, v);
      return;
   }
   ERROR("Expecting a { ?");
}

// <ITEMS> ::= "}" | <ITEM> <ITEMS>
void Items(Program *p, Output *o, Variables *v) {
   if (p->wds[p->cw][0] == '}') {
      char c = v->cv[v->loopNum];
      int pos = c - 'A';
      v->npv[pos] = v->cw[pos];
      return;
   }
   else {
      Item(p, o, v);
      Next(p);
      Items(p, o, v);
   }   
}

// <ITEM> ::= <VARNUM> | <WORD>
void Item(Program *p, Output *o, Variables *v) {
   if (p->wds[p->cw][0] == '"') {
      char c = v->cv[v->loopNum];
      int pos = c - 'A';
      Word(p, v->wds[pos][v->cw[pos]++]);
      return;
   }
   else {
      Varnum(p, o, v, 'I');
      return;
   }   
}

// <PFIX> ::= ")" | <OP> <PFIX> | <VARNUM> <PFIX>
// OP: A single mathematical operation character
// <OP> ::= + - / *
void Pfix(Program *p, Output *o, Variables *v, char *addr, int n) {
   char c = p->wds[p->cw][0];

   if (c == ')') {
      if (n != 1) {
         if (v->setIndex != 1) {
            ERROR("Invalid input for SET, please change the input file.");
         }
         strcpy(addr, v->setStack[0]);
      }
      return;
   }
   else if (c == '+' || c == '-' || c == '/' || c == '*') {
      if ((int)strlen(p->wds[p->cw]) > 1) {
         ERROR("Expecting a single mathematical operation character (+ - / *)")
      }
      calculate(v, c);
      Next(p);
      Pfix(p, o, v, addr, n);
      return;
   }
   else {
      if (n == 1) {
         strcpy(addr, p->wds[p->cw]);
      }
      else {
         Varnum(p, o, v, 'P');
         v->setIndex++;
         if (v->setIndex >= MAXNUMTOKENS) {
            ERROR("Too many tokens in SET, please change the input file.");
         }
      }
      Next(p);
      Pfix(p, o, v, addr, n);
      return;
   }
}

// Initilize the output and variables
void Init(Output *o, Variables *v) {
   o->row = ROWSTART;
   o->col = COLSTART;
   o->r[0] = ROWSTART;
   o->c[0] = COLSTART;
   o->angle = 0;
   o->pointer = 1;
   strcpy(o->colour, "\"WHITE\"");
   v->loopNum = 0;
   v->setIndex = 0;
   for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
         o->op[i][j] = ' ';
      }
   }
}

// Check if it is needed print out the picture to the screen
void is_printout(Output *o, int choice) {
   if (choice != 1) {
      o->printout = 0;
   }
   else {
      o->printout = 1;
   }
   if (o->printout == 1) {
      neillclrscrn();
   }
}

// Change a string of the colour to a letter (e.g. "RED" -> 'R')
char colour_short(char *str) {
   if (strsame(str, "\"BLACK\"")) {
      return 'K';
   }
   else if (strsame(str, "\"RED\"")) {
      return 'R';
   }
   else if (strsame(str, "\"GREEN\"")) {
      return 'G';
   }
   else if (strsame(str, "\"YELLOW\"")) {
      return 'Y';
   }
   else if (strsame(str, "\"BLUE\"")) {
      return 'B';
   }
   else if (strsame(str, "\"MAGENTA\"")) {
      return 'M';
   }
   else if (strsame(str, "\"CYAN\"")) {
      return 'C';
   }
   else if (strsame(str, "\"WHITE\"")) {
      return 'W';
   }
   else {
      ERROR("Expecting \"BLACK\" or \"RED\" or \"GREEN\" or \"YELLOW\" \
             or \"BLUE\" or \"MAGENTA\" or \"CYAN\" or \"WHITE\" ?");
   }
}

// Get the address of where to write the variable / number 
char *get_addr(Output *o, Variables *v, char flag) {
   char *addr;
   char c = v->cv[v->loopNum];
   int pos = c - 'A';

   if (flag == 'F') {
      addr = o->forward;
   }
   if (flag == 'R') {
      addr = o->degree;
   }
   if (flag == 'I') {
      addr = v->wds[pos][v->cw[pos]++];
   }
   if (flag == 'C') {
      addr = o->colour;
   }
   if (flag == 'P') {
      addr = v->setStack[v->setIndex];
   }

   return addr;
}

// Calculate when meet operator in SET instruction
void calculate(Variables *v, char c) {
   double n1, n2, res;
   char num1[MAXTOKENSIZE];
   char num2[MAXTOKENSIZE];
   char str[MAXTOKENSIZE];     

   strcpy(num1, v->setStack[v->setIndex-2]);
   strcpy(num2, v->setStack[v->setIndex-1]);
   n1 = atof(num1);
   n2 = atof(num2);
   if (c == '+') {
      res = n1 + n2;
   }
   if (c == '-') {
      res = n1 - n2;
   }
   if (c == '*') {
      res = n1 * n2;
   }
   if (c == '/') {
      res = n1 / n2;
   }
   sprintf(str, "%lf", res);
   strcpy(v->setStack[v->setIndex-1], "");
   strcpy(v->setStack[v->setIndex-2], str);
   v->setIndex--;
}

// Write the colour according to the FWD 
// instruction to the 51x33 output matrix 
void print(Output *o) {
   double distance = atof(o->forward);
   int n = (int)distance;
   
   if (o->angle == ANGLE1 || o->angle == ANGLE2 || o->angle == ANGLE3) {
      for (int i = 0; i < n; i++) {
         subprint(o);
         move(o);
      }
      record(o, distance);
   }
   else if (o->angle == ANGLE4 || o->angle == ANGLE5) {
      for (int i = 0; i < n; i++) {
         subprint(o);
         move(o);
      }
      record(o, distance);
   }
   else {
      bresenham_line(o, o->row, o->col, distance);
   }

   if (o->printout == 1) {
      print_out(o);
      neillbusywait(DELAY);
   }
}

// Record the accurate coordinates of the FWD instruction
void record(Output *o, double distance) {
   if (o->angle == ANGLE1 || o->angle == ANGLE5) {   
      o->r[o->pointer] = o->r[o->pointer-1] - distance;
      o->c[o->pointer] = o->c[o->pointer-1];
   }
   if (o->angle == ANGLE2) {
      o->r[o->pointer] = o->r[o->pointer-1];
      o->c[o->pointer] = o->c[o->pointer-1] + distance;
   }
   if (o->angle == ANGLE3) {
      o->r[o->pointer] = o->r[o->pointer-1] + distance;
      o->c[o->pointer] = o->c[o->pointer-1];
   }
   if (o->angle == ANGLE4) {
      o->r[o->pointer] = o->r[o->pointer-1];
      o->c[o->pointer] = o->c[o->pointer-1] - distance;
   }
   strcpy(o->color[o->pointer-1], o->colour);
}

// Subfunction of print()
void subprint(Output *o) {
   if (is_safe(o->row, o->col)) {
      o->op[o->row][o->col] = colour_short(o->colour);
   }
}

// Subfunction of print()
void move(Output *o) {  
   if (o->angle == ANGLE1 || o->angle == ANGLE5) {   
      o->row--;
   }
   if (o->angle == ANGLE2) {
      o->col++;
   }
   if (o->angle == ANGLE3) {
      o->row++;
   }
   if (o->angle == ANGLE4) {
      o->col--;
   }
}

// Check whethe the coordinate is safe to write in the 51x33 matrix
bool is_safe(int row, int col) {
   if (row >= HEIGHT || row < 0 || col >= WIDTH || col < 0) {
      return false;
   }
   else {
      return true;
   }
}

// Using bresenham line algorithm to determine the coordinate
// to write according to the FWD instruction
void bresenham_line(Output *o, int x1, int y1, double distance) {
   double theta = 180 - o->angle;
   double dx = distance * cos(theta * RAD);
   double dy = distance * sin(theta * RAD);
   o->r[o->pointer] = o->r[o->pointer-1] + dx;
   o->c[o->pointer] = o->c[o->pointer-1] + dy;
   strcpy(o->color[o->pointer-1], o->colour);
   int x2 = x1 + (int)dx;
   int y2 = y1 + (int)dy;
   
   if (abs(y2 - y1) < abs(x2 - x1)) {
      if (x1 > x2) {
         plot_line_low(o, x2, y2, x1, y1);
      }
      else {
         plot_line_low(o, x1, y1, x2, y2);
      }
   }
   else {
      if (y1 > y2) {
         plot_line_high(o, x2, y2, x1, y1);
      }
      else {
         plot_line_high(o, x1, y1, x2, y2);
      }  
   }
   o->row = x2;
   o->col = y2;
}

// Subfunction of bresenham_line()
void plot_line_low(Output *o, int x1, int y1, int x2, int y2) {
   int dx = x2 - x1;
   int dy = y2 - y1;
   int yi = 1;
   if (dy < 0) {
      yi = -1;
      dy = -dy;
   }
   int p = (2 * dy) - dx;
   int y = y1;   

   for (int x = x1; x <= x2 ; x++) {
      if (is_safe(x, y)) {
         o->row = x;
         o->col = y;
      }
      subprint(o);
      if (p < 0) {
         p += 2 * dy;
      } 
      else {
         p += 2 * (dy - dx);
         y = y + yi;
      }
   }
}

// Subfunction of bresenham_line()
void plot_line_high(Output *o, int x1, int y1, int x2, int y2) {
   int dx = x2 - x1;
   int dy = y2 - y1;
   int xi = 1;
   if (dx < 0) {
      xi = -1;
      dx = -dx;
   }
   int p = (2 * dx) - dy;
   int x = x1; 

   for (int y = y1; y <= y2; y++) {
      if (is_safe(x, y)) {
         o->row = x;
         o->col = y;
      }
      subprint(o);
      if (p < 0) {
         p += 2 * dx;
      } 
      else {
         p += 2 * (dx - dy);
         x = x + xi;
      }
   }
}

// Print out the picture to the screen
void print_out(Output *o) {
   neillcursorhome();
   for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
         if (o->op[i][j] == ' ') {
            printf(" ");
         }
         else {
            neillcol c = get_col(o, i, j);
            neillfgcol(c);
            neillbgcol(c);
            printf(" ");
            neillreset();
         }
      }
      printf("\n");
   }
}

// Subfunction of print_out()
neillcol get_col(Output *o, int row, int col) {
   neillcol c = white;
   if (o->op[row][col] == 'K') {
      c = black;
   }
   else if (o->op[row][col] == 'R') {
      c = red;
   }
   else if (o->op[row][col] == 'G') {
      c = green;
   }
   else if (o->op[row][col] == 'Y') {
      c = yellow;
   }
   else if (o->op[row][col] == 'B') {
      c = blue;
   }
   else if (o->op[row][col] == 'M') {
      c = magenta;
   }
   else if (o->op[row][col] == 'C') {
      c = cyan;
   }
   else {
      c = white;
   }
   return c;
}

// Output the picture to the specified file
void write_file(Output *o, char *filename, FILE *fp) {
   char *pFile = strrchr(filename, '.');

   if (strcmp(pFile, ".txt") == 0) {
      for (int i = 0; i < HEIGHT; i++) {
         for (int j = 0; j < WIDTH; j++) {
            fputc(o->op[i][j], fp);
         }
         fputc('\n', fp);
      }
      fclose(fp);
   }
   else if (strcmp(pFile, ".ps") == 0) {
      char row[MAXTOKENSIZE];
      char col[MAXTOKENSIZE];
      char color[MAXTOKENSIZE];
      char cmd[MAXNUMTOKENS];
      fputs("0.2 setlinewidth\n10 10 scale\n", fp);
      for (int i = 0; i < o->pointer-1; i++) {
         fputs("newpath\n", fp);
         sprintf(col, "%lf ", o->c[i]+COLADJUST);
         fputs(col, fp);
         sprintf(row, "%lf ", -o->r[i]+ROWADJUST);
         fputs(row, fp);
         fputs("moveto\n", fp);
         sprintf(col, "%lf ", o->c[i+1]+COLADJUST);
         fputs(col, fp);
         sprintf(row, "%lf ", -o->r[i+1]+ROWADJUST);
         fputs(row, fp);
         fputs("lineto\n", fp);
         get_color(o, i, color);
         fputs(color, fp);
         fputs("setrgbcolor\n", fp);
         fputs("stroke\n", fp);
      }
      fputs("showpage", fp);
      fclose(fp);
      strcpy(cmd, "ps2pdf ");
      strcat(cmd, filename);
      system(cmd);
   }
   else {
      fclose(fp);
   }
}

// Subfunction of write_file()
void get_color(Output *o, int i, char *color) {
   if (strsame(o->color[i], "\"BLACK\"")) {
      strcpy(color, "0 0 0 ");
   }
   else if (strsame(o->color[i], "\"RED\"")) {
      strcpy(color, "1 0 0 ");
   }
   else if (strsame(o->color[i], "\"GREEN\"")) {
      strcpy(color, "0 1 0 ");
   }
   else if (strsame(o->color[i], "\"YELLOW\"")) {
      strcpy(color, "1 1 0 ");
   }
   else if (strsame(o->color[i], "\"BLUE\"")) {
      strcpy(color, "0 0 1 ");
   }
   else if (strsame(o->color[i], "\"MAGENTA\"")) {
      strcpy(color, "1 0 1 ");
   }
   else if (strsame(o->color[i], "\"CYAN\"")) {
      strcpy(color, "0 1 1 ");
   }
   else if (strsame(o->color[i], "\"WHITE\"")) {
      // Since white ink is hard to see on white paper,
      // I’ve redefined white ink to be slightly grey.
      strcpy(color, "0.8 0.8 0.8 ");
   }
}

// Get the choice of user
int get_input_choice(void) {
   int choice = 0;

   printf("Welcome to Turtle printing program :)\n");
   printf("Please select your choice: \n");
   printf("1. I want to use an existing file\n");
   printf("2. I want to write the input file myself\n");
   printf("Input your choice: ");
   scanf("%d", &choice);  
   while (choice != 1 && choice != 2) {
      printf("Wrong input, please input again: ");
      scanf("%d", &choice); 
   }

   return choice;
}

// Get the file's name
void get_filename(char *filename) {
   char name[MAXNUMTOKENS];

   printf("Input the file name (if it is inside a folder, ");
   printf("include its folder's name as well): ");
   scanf("%s", name);
   strcpy(filename, name);
}

// Let user input a new file
void input_new_file(char *filename) {
   char name[MAXNUMTOKENS];
   char cmd[MAXNUMTOKENS];
   int lock = 1;
   
   print_grammar();
   printf("Input the name of your new file: ");
   scanf("%s", name);
   strcat(name, ".ttl");
   strcpy(filename, name);
   strcpy(cmd, "gedit ");
   strcat(cmd, name);
   system(cmd);
   printf("Input 0 to continue: ");
   while (lock != 0) {
      scanf("%d", &lock);
   }
}

// Get the output choice of user
int get_output_choice(void) {
   int choice = 0;

   printf("Please select your choice for output: \n");
   printf("1. I want to output the picture to the screen\n");
   printf("2. I want to output the picture to a .txt file\n");
   printf("3. I want to output the picture to a .ps and a .pdf file\n");
   printf("Input your choice: ");
   scanf("%d", &choice);  
   while (choice != 1 && choice != 2 && choice != 3) {
      printf("Wrong input, please input again: ");
      scanf("%d", &choice); 
   }

   return choice;
}

// Print out the grammar of Turtle printing language
void print_grammar(void) {
   printf("Here is the grammar of the Turtle printing language:\n");
   printf("<PROG>   ::= \"START\" <INSLST>\n\n");
   printf("<INSLST> ::= \"END\" | <INS> <INSLST>\n");
   printf("<INS>    ::= <FWD> | <RGT> | <COL> | <LOOP> | <SET>\n");
   printf("<FWD>    ::= \"FORWARD\" <VARNUM>\n");
   printf("<RGT>    ::= \"RIGHT\" <VARNUM>\n");
   printf("<COL>    ::= \"COLOUR\" <VAR> | \"COLOUR\" <WORD>\n");
   printf("<LOOP>   ::= \"LOOP\" <LTR> \"OVER\" <LST> <INSLST>\n");
   printf("<SET>    ::= \"SET\" <LTR> \"(\" <PFIX>\n\n");
   printf("<VARNUM> ::= <VAR> | <NUM>\n");
   printf("%% Variables e.g. $A, $B, $Z etc.\n");
   printf("<VAR>    ::= $<LTR>\n");
   printf("%% One Uppercase letter\n");
   printf("<LTR>    ::= A, B ... Z\n");
   printf("%% Any valid double (as defined by scanf(\"%%lf\"...)\n");
   printf("<NUM>    ::= 10 or -17.99 etc.\n\n");
   printf("%% A single word (as defined by scanf(\"%%s\"...) with double-quotes around it\n");
   printf("%% Valid colours include \"BLACK\", \"RED\", \"GREEN\", \"BLUE\",\n");
   printf("%% \"YELLOW\", \"CYAN\", \"MAGENTA\", \"WHITE\"\n");
   printf("<WORD>   ::= \"RED\", \"BLUE\", \"HELLO!\" or \"178\"\n");
   printf("<LST>    ::= \"{\" <ITEMS> \n");
   printf("<ITEMS>  ::= \"}\" | <ITEM> <ITEMS>\n");
   printf("<ITEM>   ::= <VARNUM> | <WORD>\n");
   printf("<PFIX>   ::= \")\" | <OP> <PFIX> | <VARNUM> <PFIX>\n");
   printf("%% A single mathematical operation character\n");
   printf("<OP>     ::= + - / *\n");
}

// Change the file extension according to the output choice
void change_filename(char *filename, int choice) {
   char name[MAXNUMTOKENS];
   char *pos;
   int len;
   
   pos = strrchr(filename, '.');
   len = pos - filename;
   strncpy(name, filename, len);
   name[len] = '\0';
   if (choice == 2) {
      strcat(name, ".txt");
   }
   if (choice == 3) {
      strcat(name, ".ps");
   }
   strcpy(filename, name);
}
