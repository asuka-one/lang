#include <termios.h>
#include <iostream>
#include <string>
#include <map>
#include <cmath>
using namespace std;

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
string text_version = "Asuka v0.0.0 - Interactive Interpreter\n";

bool to_read_line = true;
bool to_run = true;
bool to_print_usage = false;
bool to_print_version = false;

enum class State {
  SPACE,
  SYMBOL,
  NUMBER,
  FLOAT,
  PLUS,
  MINUS,
  STAR,
  SLASH
};
enum class Tag {
  END,
  OBJECT,
  OPERATOR,
  PAREN,
  ENDPAREN
};
enum class Type {
  UND,
  I64,
  F64
};
enum class Oper {
  UND,
  ADD,
  SUB,
  MUL,
  DIV
};
enum class Prec {
  ARILOW,
  ARIHIGH
};

struct Value {
  Type type;
  union {
    signed long i64;
    double f64;
  };
};

struct Expr {
  Expr* par;
  Expr* op0;
  Expr* op1;  
  Tag tag;
  Oper oper;
  Prec prec;
  Value val;
};

Expr* root;
Expr* last;

string line;
string buffer;

int line_count;

State state;
Value buffer_value;

void parse ();
Value compile (Expr*&);
void add (Expr*);
Expr* search_paren (Expr* expr);

void wrap_symbol ();
void wrap_number ();
void wrap_plus ();
void wrap_minus ();
void wrap_star ();
void wrap_slash ();
void wrap_lpar ();
void wrap_rpar ();
void wrap_end ();

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
          getchar (); // remove next '['
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
              if (!line.empty () && i < line.length ()) {
                line.erase (i, 1);
                cout << "\e[K" << line.substr (i);
                if (line.length() - i) {
                  cout << "\e[" << line.length() - i << "D";
                }
              }
              break;
          }
          break;
        case '\x7F':
          if (!line.empty () && i) {
            line.erase (--i, 1);
            cout << "\b\e[K" << line.substr (i);
            if (line.length() - i) {
              cout << "\e[" << line.length() - i << "D";
            }
          }
          break;
        default:
          line.insert(i, 1, chr);
          cout << line.substr (i++);
          if (line.length() - i) {
            cout << "\e[" << line.length() - i << "D";
          }
          break;
      }
    }
    // cout << "line: \"" << line << "\"\n";
    parse ();
  }

  return 0;
}

void parse ()
{
  int i = 0;
  for (char chr : line) {
    switch (state) {
      case State::SPACE:
        switch (chr) {
          case ' ':
            break;
          case_LOWER:
            state = State::SYMBOL;
            buffer.push_back (chr);
            break;
          case '0':
          case_NUMBER:
            buffer_value = { Type::I64,
              (signed long) (chr & ~48) };
            state = State::NUMBER;
            break;
          case '+':
            state = State::PLUS;
            break;
          case '-':
            state = State::MINUS;
            break;
          case '*':
            state = State::STAR;
            break;
          case '/':
            state = State::SLASH;
            break;
          case '(':
            wrap_lpar ();
            break;
          case ')':
            wrap_rpar ();
            break;
          default:
            break;
        }
        break;
      case State::SYMBOL:
        switch (chr) {
          default:
            wrap_symbol ();
            switch (chr) {
              case ' ':
                state = State::SPACE;
                break;
              case '+':
                state = State::PLUS;
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
      case State::NUMBER:
        switch (chr) {
          default:
            wrap_number ();
            switch (chr) {
              case ' ':
                state = State::SPACE;
                break;
              case '+':
                state = State::PLUS;
                break;
              case '-':
                state = State::MINUS;
                break;
              case '*':
                state = State::STAR;
                break;
              case '/':
                state = State::SLASH;
                break;
              case '(':
                wrap_lpar ();
                state = State::SPACE;
                break;
              case ')':
                wrap_rpar ();
                state = State::SPACE;
                break;
            }
            break;
          case '_':
            break;
          case '.':
            buffer_value.type = Type::F64;
            buffer_value.f64 = (double)buffer_value.i64;
            state = State::FLOAT;
            break;
          case_DECIMAL:
            (buffer_value.i64 *= 10) += (chr & ~48);
            break;
        }
        break;
      case State::FLOAT:
        switch (chr) {
          default:
            i = 0;
            wrap_number ();
            switch (chr) {
              case ' ':
                state = State::SPACE;
                break;
              case '+':
                state = State::PLUS;
                break;
              case '-':
                state = State::MINUS;
                break;
              case '*':
                state = State::STAR;
                break;
              case '/':
                state = State::SLASH;
                break;
              case '(':
                wrap_lpar ();
                state = State::SPACE;
                break;
              case ')':
                wrap_rpar ();
                state = State::SPACE;
                break;
            }
            break;
          case '_':
            break;
          case_DECIMAL:
            buffer_value.f64 += ((double) (chr & ~48)) / pow((double)10,(double)++i);
            break;
        }
        break;
      case State::PLUS:
        switch (chr) {
          default:
            wrap_plus ();
            switch (chr) {
              case ' ':
                state = State::SPACE;
                break;
              case '_':
              case_LOWER:
                buffer.push_back (chr);
                state = State::SYMBOL;
                break;
              case_DECIMAL:
                buffer.push_back (chr);
                state = State::NUMBER;
                break;
            }
            break;
        }
        break;
      case State::MINUS:
        switch (chr) {
          default:
            wrap_minus ();
            switch (chr) {
              case ' ':
                state = State::SPACE;
                break;
              case '_':
              case_LOWER:
                buffer.push_back (chr);
                state = State::SYMBOL;
                break;
              case_DECIMAL:
                buffer.push_back (chr);
                state = State::NUMBER;
                break;
            }
            break;
        }
        break;
      case State::STAR:
        switch (chr) {
          default:
            wrap_star ();
            switch (chr) {
              case ' ':
                state = State::SPACE;
                break;
              case '_':
              case_LOWER:
                buffer.push_back (chr);
                state = State::SYMBOL;
                break;
              case_DECIMAL:
                buffer.push_back (chr);
                state = State::NUMBER;
                break;
            }
            break;
        }
        break;
      case State::SLASH:
        switch (chr) {
          default:
            wrap_slash ();
            switch (chr) {
              case ' ':
                state = State::SPACE;
                break;
              case '_':
              case_LOWER:
                buffer.push_back (chr);
                state = State::SYMBOL;
                break;
              case_DECIMAL:
                buffer.push_back (chr);
                state = State::NUMBER;
                break;
            }
            break;
        }
        break;
    }
  }
  line_count++;
  switch (state) {
    case State::SPACE:
      break;
    case State::SYMBOL:
      wrap_symbol ();
      state = State::SPACE;
      break;
    case State::NUMBER:
    case State::FLOAT:
      wrap_number ();
      state = State::SPACE;
      break; 
  }
  wrap_end ();
  if (root) {
    Value val = compile (root);
    if (val.type == Type::UND) {
      cout << "error: types don't match\n";
      return;
    }
    cout << "  _ : ";
    switch (val.type) {
      case Type::I64:
        cout << "Int64 = " << val.i64 << '\n';
        break;
      case Type::F64:
        cout << "Float64 = " << val.f64 << '\n';
        break;
    }
  }
}

Value compile (Expr*& expr)
{
  Value tmp;
  Value vl0;
  Value vl1;
  switch (expr->tag) {
    case Tag::OBJECT:
      tmp = expr->val;
      break;
    case Tag::OPERATOR:
      if ((vl0 = compile (expr->op0)).type == Type::UND) {
        return vl0;
      }
      if ((vl1 = compile (expr->op1)).type == Type::UND) {
        return vl1;
      }
      if (vl0.type == vl1.type) {
        switch (expr->oper) {
          case Oper::ADD:
            switch (vl0.type) {
              case Type::I64:
                tmp.type = Type::I64;
                tmp.i64 = vl0.i64 + vl1.i64;
                break;
              case Type::F64:
                tmp.type = Type::F64;
                tmp.f64 = vl0.f64 + vl1.f64;
                break;
            }
            break;
          case Oper::SUB:
            switch (vl0.type) {
              case Type::I64:
                tmp.type = Type::I64;
                tmp.i64 = vl0.i64 - vl1.i64;
                break;
              case Type::F64:
                tmp.type = Type::F64;
                tmp.f64 = vl0.f64 - vl1.f64;
                break;
            }
            break;
          case Oper::MUL:
            switch (vl0.type) {
              case Type::I64:
                tmp.type = Type::I64;
                tmp.i64 = vl0.i64 * vl1.i64;
                break;
              case Type::F64:
                tmp.type = Type::F64;
                tmp.f64 = vl0.f64 * vl1.f64;
                break;
            }
            break;
          case Oper::DIV:
            switch (vl0.type) {
              case Type::I64:
                tmp.type = Type::I64;
                tmp.i64 = vl0.i64 / vl1.i64;
                break;
              case Type::F64:
                tmp.type = Type::F64;
                tmp.f64 = vl0.f64 / vl1.f64;
                break;
            }
            break;
        }
      } else {
        tmp = {Type::UND, 0};
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
    if (expr->tag == Tag::END) {
      delete expr;
      return;
    }
    root = expr;
    last = expr;
    return;
  }
  switch (last->tag) {
    case Tag::OBJECT:
      switch (expr->tag) {
        case Tag::OPERATOR:
          if (last->par) {
            if (last->par->tag == Tag::PAREN) {
              last->par->op0 = expr;
              expr->par = last->par;
              last->par = expr;
              expr->op0 = last;
              last = expr;
            } else if (last->par->prec < expr->prec) {
              expr->op0 = last;
              last->par->op1 = expr;
              expr->par = last->par;
              last->par = expr;
              last = expr;
            } else {
              if (last->par->par) {
                expr->op0 = last->par->par;
                expr->par = last->par->par->par;
                if (last->par->par->par) {
                  last->par->par->par->op0 = expr;
                }
                last->par->par->par = expr;
                if (last->par->par == root) {
                  root = expr;
                }
              } else {
                expr->op0 = last->par;
                expr->par = last->par->par;
                if (last->par->par) {
                  last->par->par->op0 = expr;
                }
                last->par->par = expr;
                if (last->par == root) {
                  root = expr;
                }
              }
              last = expr;
            }
          } else {
            expr->op0 = last;
            last->par = expr;
            if (last == root) {
              root = expr;
            }
            last = expr;
          }
          break;
        case Tag::ENDPAREN:
          last = search_paren (last);
          last->tag = Tag::ENDPAREN;
          delete expr;
          break;
        case Tag::END:
          delete expr;
          break;
      }
      break;
    case Tag::OPERATOR:
      switch (expr->tag) {
        case Tag::OBJECT:
          last->op1 = expr;
          expr->par = last;
          last = expr;
          break;
        case Tag::PAREN:
          last->op1 = expr;
          expr->par = last;
          last = expr;
          break;
      }
      break;
    case Tag::PAREN:
      switch (expr->tag) {
        case Tag::OBJECT:
          last->op0 = expr;
          expr->par = last;
          last = expr;
          break;
        case Tag::PAREN:
          last->op0 = expr;
          expr->par = last;
          last = expr;
          break;
        case Tag::ENDPAREN:
          cout << "unit???\n";
          last->tag = Tag::ENDPAREN;
          delete expr;
          break;
      }
      break;
    case Tag::ENDPAREN:
      switch (expr->tag) {
        case Tag::OPERATOR:
          if (last->par) {
            if (last->par->prec < expr->prec) {
              expr->op0 = last->op0;
              last->par->op1 = expr;
              expr->par = last->par;
              delete last;
              last = expr;
            } else {
              if (last->par->par) {
                expr->op0 = last->par->par;
                expr->par = last->par->par->par;
                if (last->par->par->par) {
                  last->par->par->par->op0 = expr;
                }
                last->par->par->par = expr;
                if (last->par->par == root) {
                  root = expr;
                }
                last->par->op1 = last->op0;
                last->op0->par = last->par;
                delete last;
                last = expr;
              } else {
                expr->op0 = last->par;
                expr->par = last->par->par;
                if (last->par->par) {
                  last->par->par->op0 = expr;
                }
                last->par->par = expr;
                if (last->par == root) {
                  root = expr;
                }
                last->par->op1 = last->op0;
                last->op0->par = last->par;
                delete last;
                last = expr;
              }
            }
          } else {
            expr->op0 = last->op0;
            last->op0->par = expr;
            root = expr;
            delete last;
            last = expr;
          }
          break;
        case Tag::ENDPAREN:
          last = search_paren (last);
          last->tag = Tag::ENDPAREN;
          delete expr;
          break;
        case Tag::END:
          if (last == root) {
            root = last->op0;
          }
          if (last->par) {
            last->par->op1 = last->op0;
            if (last->op0) {
              last->op0->par = last->par;
            }
          }
          delete last;
          delete expr;
          break;
      }
      break;
  }
}

Expr* search_paren (Expr* expr)
{
  if (expr->tag == Tag::PAREN) {
    return expr;
  }
  if (expr->par) {
    return search_paren (expr->par);
  }
  return 0;
}

void wrap_symbol ()
{
  cout << "symbol: " << buffer << '\n';
  buffer.clear ();
}

void wrap_number ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OBJECT;
  expr->val = buffer_value;
  buffer_value = {};
  add (expr);
}

void wrap_plus ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::ADD;
  expr->prec = Prec::ARILOW;
  add (expr);
}

void wrap_minus ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::SUB;
  expr->prec = Prec::ARILOW;
  add (expr);
}

void wrap_star ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::MUL;
  expr->prec = Prec::ARIHIGH;
  add (expr);
}

void wrap_slash ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::DIV;
  expr->prec = Prec::ARIHIGH;
  add (expr);
}

void wrap_lpar ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::PAREN;
  add (expr);
}

void wrap_rpar ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::ENDPAREN;
  add (expr);
}

void wrap_end ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::END;
  add (expr);
}

