//g++ main.cpp -o main

#include <unordered_map>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
using namespace std;

#define case_LOW \
  case'a':case'b':case'c': \
  case'd':case'e':case'f': \
  case'g':case'h':case'i': \
  case'j':case'k':case'l': \
  case'm':case'n':case'o': \
  case'p':case'q':case'r': \
  case's':case't':case'u': \
  case'v':case'w':case'x': \
  case'y':case'z'
#define case_CAP \
  case'A':case'B':case'C': \
  case'D':case'E':case'F': \
  case'G':case'H':case'I': \
  case'J':case'K':case'L': \
  case'M':case'N':case'O': \
  case'P':case'Q':case'R': \
  case'S':case'T':case'U': \
  case'V':case'W':case'X': \
  case'Y':case'Z'
#define case_BIN \
  case'0':case'1'
#define case_OCT \
  case_BIN:case'2':case'3': \
  case '4':case'5':case'6': \
  case '7'
#define case_DEC \
  case_OCT:case'8':case'9'
#define case_HEX \
  case_DEC:case'A':case'B': \
  case 'C':case'D':case'E': \
  case 'F'
#define case_NUM \
  case'1':case'2':case'3': \
  case'4':case'5':case'6': \
  case'7':case'8':case'9'
#define case_OPR \
  case':':case'=':case'+': \
  case'-':case'*':case'/': \
  case'<':case'>':case'.': \
  case'&':case'|':case'~': \
  case'!':case'?':case'$'

enum Type { EXPR = 1, NUMBER, STRING, SYMBOL };
enum Prec {
  PNONE, JUXTA, COMPO, TPMAP, UNARY,
  BWAND, BWXOR, BWIOR, ARIMD,
  ARIAS, CONCA, ASIGN, LOREL,
  COMPA
};
enum Oper { NONE, PLUS, MINUS, MULT, DIVD };
 
unordered_map<string, Oper> oprs = {
  {">>",NONE},{"->",NONE},{"+", PLUS},{"-",MINUS},
  {"~", NONE},{"?", NONE},{"!", NONE},{"$", NONE},
  {"&", NONE},{"|", NONE},{"*", MULT},{"/", DIVD},
  {":-",NONE},{":=",NONE},{"+=",NONE},{"-=",NONE},
  {"*=",NONE},{"/=",NONE},{"&=",NONE},{"~=",NONE},
  {"|=",NONE},{"<", NONE},{">", NONE},{"<=",NONE},
  {">=",NONE},{"=", NONE},{"!=",NONE}
};
unordered_map<Oper,Prec> preces = {
  {PLUS,ARIAS},{MINUS,ARIAS},
  {MULT,ARIMD},{DIVD,ARIMD}
};
unordered_map<string,int> kwds = {
  {"And",    1},{"Break",   2},
  {"Do",     3},{"Echo",    4},
  {"Else",   5},{"For",     6},
  {"If",     7},{"In",      8},
  {"Is",     9},{"Matches",10},
  {"New",   11},{"Next",   12},
  {"Not",   13},{"Or",     14},
  {"Raw",   15},{"Redo",   16},
  {"Return",17},{"Then",   18},
  {"Use",   19},{"__exit", 20},
  {"__line",23}
}; 

struct Object {
  Object* sup;
  vector<Object*> objs;
  long long value;
  Type type;
  Oper oper;
  Prec prec;
};
Object* expr;

string outpath = "out";
bool run = true;

enum State {
  LINE, COML, SYMB, BSPC,
  OPER, ZERO, BINN, OCTN,
  DECN, HEXN
};
State state;

int line_cnt  = 0, expr_nest = 0,
    comm_nest = 0, indt_nest = 0;
vector<int> indnt = {0};
string buf_str = "", buf_sym = "",
       buf_num = "", buf_spc = "",
       buf_opr = "";

void eval_sym (char chr) {    
  switch (kwds[buf_sym]) {
    case 20  : run = false; break;
    case 23  : cout << line_cnt << "\n"; break;
    default  : break;
  } buf_sym.clear();
  switch (chr) {
    case ' ' : state = BSPC; break;
    case_OPR : buf_opr.push_back(chr); state = OPER; break;
    default  : break; 
  }
}

void eval_opr (char chr) {    
  if (oprs[buf_opr]) switch (expr->type) {
    case NUMBER:
      expr = new Object {0,{expr},0,EXPR,
        oprs[buf_opr],preces[oprs[buf_opr]]};
      expr->objs[0]->sup = expr; break;
    case EXPR  :
      expr = new Object {0,{expr},0,EXPR,
        oprs[buf_opr],preces[oprs[buf_opr]]};
      expr->objs[0]->sup = expr; break; 
  } buf_opr.clear();
  switch (chr) {
    case ' ' : state = BSPC; break;
    case_LOW : buf_sym.push_back(chr); state = SYMB; break;
    default  : break;
  }
}

long long eval_exp (Object*& obj) {
  long long result = 0;
  switch (obj->type) {
    case EXPR: switch (obj->oper) {
      case PLUS:
        result = eval_exp(obj->objs[0]) + eval_exp(obj->objs[1]);
        delete obj->objs[0]; delete obj->objs[1]; obj->objs.shrink_to_fit();
        break;
      case MINUS:
        result = eval_exp(obj->objs[0]) - eval_exp(obj->objs[1]);
        delete obj->objs[0]; delete obj->objs[1]; obj->objs.shrink_to_fit(); 
        break;
      case MULT:
        result = eval_exp(obj->objs[0]) * eval_exp(obj->objs[1]);
        delete obj->objs[0]; delete obj->objs[1]; obj->objs.shrink_to_fit();
        break;
      case DIVD:
        result = eval_exp(obj->objs[0]) / eval_exp(obj->objs[1]);
        delete obj->objs[0]; delete obj->objs[1]; obj->objs.shrink_to_fit();
        break;
    } break;
    case NUMBER: result = obj->value;
  }
  obj = 0;
  return result;
}

void eval_num (char chr) {
  long long value = 0;
  switch (state) {
    case BINN: for (char c : buf_num)
      (value *= 2) += c - 48; break;
    case OCTN: for (char c : buf_num)
      (value *= 8) += c - 48; break;
    case DECN: for (char c : buf_num)
      (value *= 10) += c - 48; break;
    case HEXN: for (char c : buf_num)
      (value *= 16) += (c > 57) ? c - 65 : c - 48; break;
  } buf_num.clear();
  switch (chr) {
    case ' ' : state = BSPC; break;
    default  : break;
  } if (!expr) expr = new Object {0,{},value,NUMBER,NONE,PNONE};
  else if (expr->type == EXPR) switch (expr->oper) {
    case PLUS:
      expr->objs.push_back( new Object
        {0,{},value,NUMBER,NONE,PNONE} );
      break;
    case MINUS:
      expr->objs.push_back( new Object
        {0,{},value,NUMBER,NONE,PNONE} );
      break;
    case MULT:
      expr->objs.push_back( new Object
        {0,{},value,NUMBER,NONE,PNONE} );
      break;
    case DIVD:
      expr->objs.push_back( new Object
        {0,{},value,NUMBER,NONE,PNONE} );
      break;  
  }
}

void parse_line(string line) { 
  state = LINE; line_cnt += 1;
  for (char chr  : line) {
    switch (state) {

      case LINE  : switch (chr) {
        case ' ' : buf_spc.push_back(chr); break;
        case '#' : buf_spc.clear(); state = COML; break;
        case '0' : state = ZERO; break;
        case_NUM : buf_spc.clear(); buf_num.push_back(chr); state = DECN; break;
        case '_' :
        case_LOW : buf_spc.clear(); buf_sym.push_back(chr); state = SYMB; break;
      } break;
      
      case COML  : break;

      case SYMB  : switch (chr) {
        default  : eval_sym(chr); break;
        case '_' :
        case_LOW : buf_sym.push_back(chr); break;
      } break;
      
      case BSPC  : switch (chr) {
        case ' ' : break;
        case_OPR : buf_opr.push_back(chr); state = OPER; break;
        case_LOW : buf_sym.push_back(chr); state = SYMB; break;
        case_NUM : buf_num.push_back(chr); state = DECN; break;
      } break;

      case OPER  : switch (chr) {
        default  : eval_opr(chr); break;
        case_OPR : buf_opr.push_back(chr); break;
      } break;

      case ZERO  : switch (chr) {
        case 'b' : state = BINN; break;
        case 'o' : state = OCTN; break;
        case 'h' : state = HEXN; break;
      } break;

      case BINN  : switch (chr) {
        default  : eval_num(chr); break;
        case_BIN : buf_num.push_back(chr); break;
      } break;

      case OCTN  : switch (chr) {
        default  : eval_num(chr); break;
        case_OCT : buf_num.push_back(chr); break;
      } break;

      case DECN  : switch (chr) {
        default  : eval_num(chr); break;
        case_DEC : buf_num.push_back(chr); break;
      } break; 

      case HEXN  : switch (chr) {
        default  : eval_num(chr); break;
        case_HEX : buf_num.push_back(chr); break;
      } break;

      default    : break;
    }
  }

  switch (state) {
    case LINE:
    case COML:
    case BSPC: break;
    case SYMB: eval_sym('\n'); break;
    case DECN: eval_num('\n'); break;
  }
  if (expr) cout << eval_exp(expr) << "\n";
}

vector<string>* parse_args(int argc, char** argv) {
  const string USG = "help will be soon";
  const string VER = "0.0.1";
  bool f_usg = false, f_ver = false, tmp = false;
  auto files = new vector<string>();
  
  static unordered_map<string,int> args = {
    {"--help",    1}, {"-h", 1},
    {"--version", 2}, {"-v", 2},
    {"-o", 3}
  };

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (args[argv[i]]) {
        case 1: f_usg = true; break;
        case 2: f_ver = true; break;
        case 3: tmp = true; break;
        default: break;
      }    
    } else if (tmp) {
      outpath = argv[i]; tmp = false;
    } else {
      files->push_back(argv[i]);
    }
  }

  if (f_usg) { cout << USG << endl; exit(0); }
  if (f_ver) { cout << VER << endl; exit(0); }
  return files;
}
 
int main(int argc, char** argv) {
  string line = "";
  vector<string>* paths = parse_args(argc, argv);
  if (paths->size() > 0) {
    // vector<string> contents = {};
    for (auto path : *paths) cout << path << "\n";
    return 1;
  } else {
    while (run) { cout << "> "; getline(cin,line); parse_line(line); }
  }
  return 0;
}
