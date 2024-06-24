#include <termios.h>
#include <iostream>
#include <string>
#include <map>
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
#define MAXDIGITPOS 1000000000000000 // 10 ** 15

map <string, int> args = {
  {"--help", 1},
  {"-h", 1},
  {"--version", 2},
  {"-v", 2}
};

map <string, int> kwds = {
  {"true", 1},
  {"false", 2}
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
  DECIMAL,
  FLOAT,
  PLUS,
  MINUS,
  STAR,
  SLASH,
  BAR,
  TILDE,
  AMPER
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
  F64,
  BOOL
};
enum class Oper {
  UND,
  ADD,
  SUB,
  MUL,
  DIV,
  IOR,
  XOR,
  AND
};
enum class Prec {
  ARILOW,
  ARIHIGH,
  BITOR,
  BITXOR,
  BITAND
};

struct Value {
  Type type;
  union {
    signed long i64;
    double f64;
    bool bl;
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
struct Statement {
  Expr* root;
  Expr* last;
};

string line;
string buffer;

int line_count;

State state;
Value buffer_value;
Statement stmt;

void read_line ();
void parse ();
Value compile (Expr*&);
Expr* search_paren (Expr*);

void wrap_symbol ();
void wrap_number ();
void wrap_bool ();
void wrap_plus ();
void wrap_minus ();
void wrap_star ();
void wrap_slash ();
void wrap_or ();
void wrap_xor ();
void wrap_and ();
void wrap_lpar ();
void wrap_rpar ();
void wrap_end ();

void print_error (string);
void clear (Value&);
void clear (Statement&);

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
    read_line ();
    parse ();
  }

  return 0;
}

void read_line ()
{
  line.clear();
  int index = 0;
  cout << text_prompt_prefix;
loop:
  char chr = getchar ();
  switch (chr) {
    case '\n':
      cout << '\n';
      return;
    case '\e':
      getchar (); // remove next '['
      switch (getchar()) {
        case 'A':
          goto loop;
        case 'B':
          goto loop;
        case 'C':
          if (index < line.length ()) {
            index++;
            cout << "\e[C";
          }
          goto loop;
        case 'D':
          if (index) {
            index--;
            cout << "\e[D";
          }
          goto loop;
        case '3':
          getchar (); // remove next '~'
          if (!line.empty () && index < line.length ()) {
            line.erase (index, 1);
            cout << "\e[K" << line.substr (index);
            if (line.length() - index) {
              cout << "\e[" << line.length() - index << "D";
            }
          }
          goto loop;
      }
    case '\x7F':
      if (!line.empty () && index) {
        line.erase (--index, 1);
        cout << "\b\e[K" << line.substr (index);
        if (line.length() - index) {
          cout << "\e[" << line.length() - index << "D";
        }
      }
      goto loop;
    default:
      line.insert (index, 1, chr);
      cout << line.substr (index++);
      if (line.length() - index) {
        cout << "\e[" << line.length() - index << "D";
      }
      goto loop;
  }
}

void parse ()
{
  static long digit_pos = 1;
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
          case '|':
            state = State::BAR;
            break;
          case '~':
            state = State::TILDE;
            break;
          case '&':
            state = State::AMPER;
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
            state = State::DECIMAL;
            break;
          case_DECIMAL:
            (buffer_value.i64 *= 10) += (chr & ~48);
            break;
        }
        break;
      case State::DECIMAL:
        switch (chr) {
          default:
            print_error ("malformed floating point number");
            // clear_stmt (stmt);
            state = State::SPACE;
            break;
          case_DECIMAL:
            buffer_value.type = Type::F64;
            buffer_value.f64 = (double)buffer_value.i64;
            buffer_value.f64 +=
              (double)(chr & ~48) /
              (double)(digit_pos *= 10);
            state = State::FLOAT;
            break;
        }
        break;
      case State::FLOAT:
        switch (chr) {
          default:
            wrap_number ();
            digit_pos = 1;
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
            if (digit_pos >= MAXDIGITPOS) {
              print_error ("floating point number is too long");
              digit_pos = 1;
              clear (buffer_value);
              clear (stmt);
              break;
            }
            buffer_value.f64 +=
              (double)(chr & ~48) /
              (double)(digit_pos *= 10);
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
      case State::BAR:
        switch (chr) {
          default:
            wrap_or ();
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
      case State::TILDE:
        switch (chr) {
          default:
            wrap_xor ();
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
      case State::AMPER:
        switch (chr) {
          default:
            wrap_and ();
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
      digit_pos = 1;
      state = State::SPACE;
      break; 
  }
  wrap_end ();
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
          case Oper::IOR:
            switch (vl0.type) {
              case Type::I64:
                tmp.type = Type::I64;
                tmp.i64 = vl0.i64 | vl1.i64;
                break;
              case Type::BOOL:
                tmp.type = Type::BOOL;
                tmp.bl = vl0.bl | vl1.bl;
                break;
            }
            break;
          case Oper::XOR:
            switch (vl0.type) {
              case Type::I64:
                tmp.type = Type::I64;
                tmp.i64 = vl0.i64 ^ vl1.i64;
                break;
              case Type::BOOL:
                tmp.type = Type::BOOL;
                tmp.bl = vl0.bl ^ vl1.bl;
                break;
            }
            break;
          case Oper::AND:
            switch (vl0.type) {
              case Type::I64:
                tmp.type = Type::I64;
                tmp.i64 = vl0.i64 & vl1.i64;
                break;
              case Type::BOOL:
                tmp.type = Type::BOOL;
                tmp.bl = vl0.bl & vl1.bl;
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

void add_object (Expr* expr)
{
  if (!stmt.root) {
    stmt.root = expr;
    stmt.last = expr;
    return;
  }
  switch (stmt.last->tag) {
    case Tag::OPERATOR:
      stmt.last->op1 = expr;
      expr->par = stmt.last;
      stmt.last = expr;
      break;
    case Tag::PAREN:
      stmt.last->op0 = expr;
      expr->par = stmt.last;
      stmt.last = expr;
      break;
  }
}

void add_oper (Expr* expr)
{
  switch (stmt.last->tag) {
    case Tag::OBJECT:
      if (stmt.last->par) {
        if (stmt.last->par->tag == Tag::PAREN) {
          stmt.last->par->op0 = expr;
          expr->par = stmt.last->par;
          stmt.last->par = expr;
          expr->op0 = stmt.last;
          stmt.last = expr;
        } else if (stmt.last->par->prec < expr->prec) {
          expr->op0 = stmt.last;
          stmt.last->par->op1 = expr;
          expr->par = stmt.last->par;
          stmt.last->par = expr;
          stmt.last = expr;
        } else {
          if (stmt.last->par->par) {
            expr->op0 = stmt.last->par->par;
            expr->par = stmt.last->par->par->par;
            if (stmt.last->par->par->par) {
              stmt.last->par->par->par->op0 = expr;
            }
            stmt.last->par->par->par = expr;
            if (stmt.last->par->par == stmt.root) {
              stmt.root = expr;
            }
          } else {
            expr->op0 = stmt.last->par;
            expr->par = stmt.last->par->par;
            if (stmt.last->par->par) {
              stmt.last->par->par->op0 = expr;
            }
            stmt.last->par->par = expr;
            if (stmt.last->par == stmt.root) {
              stmt.root = expr;
            }
          }
          stmt.last = expr;
        }
      } else {
        expr->op0 = stmt.last;
        stmt.last->par = expr;
        if (stmt.last == stmt.root) {
          stmt.root = expr;
        }
        stmt.last = expr;
      }
      break;
    case Tag::ENDPAREN:
      if (stmt.last->par) {
        if (stmt.last->par->prec < expr->prec) {
          expr->op0 = stmt.last->op0;
          stmt.last->par->op1 = expr;
          expr->par = stmt.last->par;
          delete stmt.last;
          stmt.last = expr;
        } else {
          if (stmt.last->par->par) {
            expr->op0 = stmt.last->par->par;
            expr->par = stmt.last->par->par->par;
            if (stmt.last->par->par->par) {
              stmt.last->par->par->par->op0 = expr;
            }
            stmt.last->par->par->par = expr;
            if (stmt.last->par->par == stmt.root) {
              stmt.root = expr;
            }
            stmt.last->par->op1 = stmt.last->op0;
            stmt.last->op0->par = stmt.last->par;
            delete stmt.last;
            stmt.last = expr;
          } else {
            expr->op0 = stmt.last->par;
            expr->par = stmt.last->par->par;
            if (stmt.last->par->par) {
              stmt.last->par->par->op0 = expr;
            }
            stmt.last->par->par = expr;
            if (stmt.last->par == stmt.root) {
              stmt.root = expr;
            }
            stmt.last->par->op1 = stmt.last->op0;
            stmt.last->op0->par = stmt.last->par;
            delete stmt.last;
            stmt.last = expr;
          }
        }
      } else {
        expr->op0 = stmt.last->op0;
        stmt.last->op0->par = expr;
        stmt.root = expr;
        delete stmt.last;
        stmt.last = expr;
      }
      break;
  }
}

void wrap_symbol ()
{
  switch (kwds[buffer]) {
    case 0:
      cout << "symbol: " << buffer << '\n';
      break;
    case 1:
      buffer_value.type = Type::BOOL;
      buffer_value.bl = true;
      wrap_bool ();
      break;
    case 2:
      buffer_value.type = Type::BOOL;
      buffer_value.bl = false;
      wrap_bool ();
      break;
  }
  buffer.clear ();
}

void wrap_number ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OBJECT;
  expr->val = buffer_value;
  clear (buffer_value);
  add_object (expr);
}

void wrap_bool ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OBJECT;
  expr->val = buffer_value;
  clear (buffer_value);
  add_object (expr);
}

void wrap_plus ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::ADD;
  expr->prec = Prec::ARILOW;
  add_oper (expr);
}

void wrap_minus ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::SUB;
  expr->prec = Prec::ARILOW;
  add_oper (expr);
}

void wrap_star ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::MUL;
  expr->prec = Prec::ARIHIGH;
  add_oper (expr);
}

void wrap_slash ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::DIV;
  expr->prec = Prec::ARIHIGH;
  add_oper (expr);
}

void wrap_or ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::IOR;
  expr->prec = Prec::BITOR;
  add_oper (expr);
}

void wrap_xor ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::XOR;
  expr->prec = Prec::BITXOR;
  add_oper (expr);
}

void wrap_and ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::OPERATOR;
  expr->oper = Oper::AND;
  expr->prec = Prec::BITAND;
  add_oper (expr);
}

void wrap_lpar ()
{
  Expr* expr = new Expr ();
  expr->tag = Tag::PAREN;
  if (!stmt.root) {
    stmt.root = expr;
    stmt.last = expr;
    return;
  }
  switch (stmt.last->tag) {
    case Tag::OPERATOR: 
      stmt.last->op1 = expr;
      expr->par = stmt.last;
      stmt.last = expr;
      break;
    case Tag::PAREN:
      stmt.last->op0 = expr;
      expr->par = stmt.last;
      stmt.last = expr;
      break;
  }
}

void wrap_rpar ()
{
  switch (stmt.last->tag) {
    case Tag::OBJECT:
      stmt.last = search_paren (stmt.last);
      stmt.last->tag = Tag::ENDPAREN;
      break;
    case Tag::PAREN:
      cout << "unit???\n";
      stmt.last->tag = Tag::ENDPAREN;
      break;
    case Tag::ENDPAREN:
      stmt.last = search_paren (stmt.last);
      stmt.last->tag = Tag::ENDPAREN;
      break;
  }
}

void wrap_end ()
{
  if (!stmt.root) {
    return;
  }
  switch (stmt.last->tag) {   
    case Tag::OBJECT:
      break;
    case Tag::ENDPAREN:
      if (stmt.last == stmt.root) {
        stmt.root = stmt.last->op0;
      }
      if (stmt.last->par) {
        stmt.last->par->op1 = stmt.last->op0;
        if (stmt.last->op0) {
          stmt.last->op0->par = stmt.last->par;
        }
      }
      delete stmt.last;
      break;
  }
  Value val = compile (stmt.root);
  if (val.type == Type::UND) {
    cout << "error: types don't match\n";
    return;
  }
  cout << "  _ : ";
  switch (val.type) {
    case Type::I64:
      cout << "Int64 = "
        << val.i64
        << '\n';
      break;
    case Type::F64:
      cout << "Float64 = "
        << val.f64
        << '\n';
      break;
    case Type::BOOL:
      cout << "Bool = "
        << val.bl
        << '\n';
      break;
  }
}

void print_error (string msg)
{
  cerr << "error: " << msg << "\n";
}

void clear (Value& value)
{
  value = Value ();
}

void clear (Statement& stmt)
{
  stmt = Statement ();
}

