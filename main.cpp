#include <unistd.h>
#include <termios.h>
#include <iostream>
#include <list>
#include <string>
#include <map>
using namespace std;

/*
  TODO:
  + add mul and div operators
    (implies creating priority system)
  - add floating point literals
    (implies creating type system)
  - add parens grouping
  - add variables
  - add type signatures
*/

#define case_LOWER \
  case'a':case'b':case'c': \
  case'd':case'e':case'f': \
  case'g':case'h':case'i': \
  case'j':case'k':case'l': \
  case'm':case'n':case'o': \
  case'p':case'q':case'r': \
  case's':case't':case'u': \
  case'v':case'w':case'x': \
  case'y':case'z'
#define case_UPPER \
  case'A':case'B':case'C': \
  case'D':case'E':case'F': \
  case'G':case'H':case'I': \
  case'J':case'K':case'L': \
  case'M':case'N':case'O': \
  case'P':case'Q':case'R': \
  case'S':case'T':case'U': \
  case'V':case'W':case'X': \
  case'Y':case'Z'
#define case_BINARY \
  case'0':case'1'
#define case_OCT \
  case_BINARY:case'2':case'3': \
  case '4':case'5':case'6': \
  case '7'
#define case_HEXADECIMAL \
  case_DECIMAL:case'A':case'B': \
  case 'C':case'D':case'E': \
  case 'F'
#define case_NUMBER \
  case'1':case'2':case'3': \
  case'4':case'5':case'6': \
  case'7':case'8':case'9'
#define case_DECIMAL \
  case'0':case_NUMBER

map <string, int> args = {
  {"--help", 1},
  {"-h", 1},
  {"--version", 2},
  {"-v", 2}
};

string text_prompt_prefix = "> ";
string text_usage = "Usage: comp [options] [files]\n"
  "Options:\n"
  "  -h, --help     Display this text\n"
  "  -v, --version  Display compiler information\n";
string text_version = "Asuka v0.0.1 - Interactive Incremental Compiler\n";

bool to_read_line = true;
bool to_run = true;
bool to_print_usage = false;
bool to_print_version = false;

enum {
  SPACE,
  SYMBOL,
  NUMBER,
  PLUS,
  MINUS,
  STAR,
  SLASH
} state = SPACE;
enum ExprTag {
  E_UND,
  E_OBJ,
  E_OPER
};
enum ValueType {
  T_UND,
  T_I64,
  T_F64
};
enum ExprOper {
  O_UND,
  O_ADD,
  O_SUB,
  O_MUL,
  O_DIV
};
enum ExprPrec {
  ARILOW,
  ARIHIGH
};

struct Value {
  ValueType type;
  union {
    signed long i64;
  };
};

struct Expr {
  ExprTag tag;
  struct {
    ExprOper oper;
    ExprPrec prec;
  };
  Expr* par;
  Expr* op0;
  Expr* op1;
  Value val;
};

Expr* root;
Expr* last;

string line;
string buffer;

int line_count;

void parse ();
Value compile (Expr*&);
void add (Expr*);

void wrap_symbol ();
void wrap_number ();
void wrap_plus ();
void wrap_minus ();
void wrap_star ();
void wrap_slash ();

int main (int argc, char** argv)
{
  for (int i = 0; i < argc; i++) {
    switch (args[argv[i]]) {
      case 1:
        to_print_usage = true;
        break;
      case 2:
        to_print_version = true;
        break;        
    }
  }

  if (to_print_usage) {
    cout << text_usage;
    return 0;
  }

  if (to_print_version) {
    cout << text_version;
    return 0;
  }

  termios term;
  tcgetattr (0, &term);
  term.c_lflag &= ~ICANON;
  term.c_lflag &= ~ECHO;
  tcsetattr (0, TCSANOW, &term);
  
  cout << text_version;

  while (to_run) {
    to_read_line = true;
    line.clear();
    int i = 0;
    cout << text_prompt_prefix;
    while (to_read_line) {
      char chr = getchar ();
      switch (chr) {
        case '\n':
          to_read_line = false;
          cout << '\n';
          break;
        case '\e':
          if (getchar () == '[') {
            switch (getchar()) {
              case 'A':
                break;
              case 'B':
                break;
              case 'C':
                if (i < line.length ()) {
                  i++;
                  cout << "\e[C";
                }
                break;
              case 'D':
                if (i != 0) {
                  i--;
                  cout << "\e[D";
                }
                break;
              case '3':
                getchar (); // remove next '~'
                if (!line.empty ()) {
                  line.erase (i, 1);
                  cout << "\e[K" << line.substr (i) <<
                    "\e[" + to_string(line.length() - i) + "D";
                }
                break;
            }
          }
          break;
        case '\x7F':
          if (!line.empty ()) {
            line.pop_back ();
            i--;
            cout << "\b \b";
          }
          break;
        default:
          line.push_back(chr);
          i++;
          cout << chr;
          break;
      }
    }
    parse ();
  }

  return 0;
}

void parse ()
{
  for (char chr : line) {
    switch (state) {
      case SPACE:
        switch (chr) {
          case ' ':
            break;
          case_LOWER:
            state = SYMBOL;
            buffer.push_back (chr);
            break;
          case '0':
          case_NUMBER:
            buffer.push_back (chr);
            state = NUMBER;
            break;
          case '+':
            state = PLUS;
            break;
          case '-':
            state = MINUS;
            break;
          case '*':
            state = STAR;
            break;
          case '/':
            state = SLASH;
            break;
          default:
            break;
        }
        break;
      case SYMBOL:
        switch (chr) {
          default:
            wrap_symbol ();
            switch (chr) {
              case ' ':
                state = SPACE;
                break;
              case '+':
                state = PLUS;
                break;
            }
            break;
          case '_':
          case_LOWER:
          case_DECIMAL:
            buffer.push_back (chr);
            break;
        }
        break;
      case NUMBER:
        switch (chr) {
          default:
            wrap_number ();
            switch (chr) {
              case ' ':
                state = SPACE;
                break;
              case '+':
                state = PLUS;
                break;
              case '-':
                state = MINUS;
                break;
            }
            break;
          case '_':
            break;
          case_DECIMAL:
            buffer.push_back (chr);
            break;
        }
        break;
      case PLUS:
        switch (chr) {
          default:
            wrap_plus ();
            switch (chr) {
              case ' ':
                state = SPACE;
                break;
              case '_':
              case_LOWER:
                buffer.push_back (chr);
                state = SYMBOL;
                break;
              case_DECIMAL:
                buffer.push_back (chr);
                state = NUMBER;
                break;
            }
            break;
        }
        break;
      case MINUS:
        switch (chr) {
          default:
            wrap_minus ();
            switch (chr) {
              case ' ':
                state = SPACE;
                break;
              case '_':
              case_LOWER:
                buffer.push_back (chr);
                state = SYMBOL;
                break;
              case_DECIMAL:
                buffer.push_back (chr);
                state = NUMBER;
                break;
            }
            break;
        }
        break;
      case STAR:
        switch (chr) {
          default:
            wrap_star ();
            switch (chr) {
              case ' ':
                state = SPACE;
                break;
              case '_':
              case_LOWER:
                buffer.push_back (chr);
                state = SYMBOL;
                break;
              case_DECIMAL:
                buffer.push_back (chr);
                state = NUMBER;
                break;
            }
            break;
        }
        break;
      case SLASH:
        switch (chr) {
          default:
            wrap_slash ();
            switch (chr) {
              case ' ':
                state = SPACE;
                break;
              case '_':
              case_LOWER:
                buffer.push_back (chr);
                state = SYMBOL;
                break;
              case_DECIMAL:
                buffer.push_back (chr);
                state = NUMBER;
                break;
            }
            break;
        }
        break;
    }
  }
  line_count++;
  switch (state) {
    case SPACE:
      break;
    case SYMBOL:
      wrap_symbol ();
      state = SPACE;
      break;
    case NUMBER:
      wrap_number ();
      state = SPACE;
      break; 
  }
  if (root) {
    Value val = compile (root);
    cout << "  _ : ";
    switch (val.type) {
      case T_I64:
        cout << "Int64 = " << val.i64 << '\n';
        break;
    }
  }
}

Value compile (Expr*& expr)
{
  Value tmp;
  switch (expr->tag) {
    case E_OBJ:
      tmp = expr->val;
      break;
    case E_OPER:
      switch (expr->oper) {
        case O_ADD:
          tmp = {T_I64, compile(expr->op0).i64 +
            compile(expr->op1).i64};
          break;
        case O_SUB:
          tmp = {T_I64, compile(expr->op0).i64 -
            compile(expr->op1).i64};
          break;
        case O_MUL:
          tmp = {T_I64, compile(expr->op0).i64 *
            compile(expr->op1).i64};
          break;
        case O_DIV:
          tmp = {T_I64, compile(expr->op0).i64 /
            compile(expr->op1).i64};
          break;
      }
      break;
  }
  delete expr;
  expr = 0;
  return tmp;
}

void add (Expr* expr)
{
  if (!root) {
    root = expr;
    last = expr;
    return;
  }
  switch (last->tag) {
    case E_OBJ:
      switch (expr->tag) {
        case E_OPER:
          if (last->par) {
            if (last->par->prec < expr->prec) {
              expr->op0 = last->par->op1;
              last->par->op1 = expr;
              expr->par = last->par;
              last = expr;
            } else if (last->par->prec >= expr->prec) {
              if (last->par->par) {
                expr->op0 = last->par->par;
              } else {
                expr->op0 = last->par;
              }
              root = expr;
              last = expr;
            }
          } else {
            expr->op0 = last;
            last->par = expr;
            root = expr;
            last = expr;
          }
          break;
      }
      break;
    case E_OPER:
      switch (expr->tag) {
        case E_OBJ:
          last->op1 = expr;
          expr->par = last;
          last = expr;
          break;
      }
      break;
  }
}

void wrap_symbol ()
{
  cout << "symbol: " << buffer << '\n';
  buffer.clear ();
}

void wrap_number ()
{
  Expr* expr = new Expr ();
  expr->tag = E_OBJ;
  expr->val.type = T_I64;
  for (char chr : buffer) {
    (expr->val.i64 *= 10) += (chr - 48);
  }
  buffer.clear ();
  add (expr);
}

void wrap_plus ()
{
  Expr* expr = new Expr ();
  expr->tag = E_OPER;
  expr->oper = O_ADD;
  expr->prec = ARILOW;
  add (expr);
}

void wrap_minus ()
{
  Expr* expr = new Expr ();
  expr->tag = E_OPER;
  expr->oper = O_SUB;
  expr->prec = ARILOW;
  add (expr);
}

void wrap_star ()
{
  Expr* expr = new Expr ();
  expr->tag = E_OPER;
  expr->oper = O_MUL;
  expr->prec = ARIHIGH;
  add (expr);
}

void wrap_slash ()
{
  Expr* expr = new Expr ();
  expr->tag = E_OPER;
  expr->oper = O_DIV;
  expr->prec = ARIHIGH;
  add (expr);
}

