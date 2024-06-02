
#include ""

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

char* USG = "help will be soon\n";
char* VER = "0.0.1\n";
char* prompt_prefix = "> ";

struct TypeDesc {
  int size;
  int type;
  int fnum;
  struct {
    Str* name;
    int type;
  } flst[];
};

#if 0

Map_Str_U8 args = {
  {"--help",    1}, {"-h", 1},
  {"--version", 2}, {"-v", 2},
  {"-o", 3}
};

Map_Str_U8 oprs = {
  {">>",NONE},{"->",NONE},{"+", PLUS},{"-",MINUS},
  {"~", NONE},{"?", NONE},{"!", NONE},{"$", NONE},
  {"&", NONE},{"|", NONE},{"*", MULT},{"/", DIVD},
  {":-",NONE},{":=",NONE},{"+=",NONE},{"-=",NONE},
  {"*=",NONE},{"/=",NONE},{"&=",NONE},{"~=",NONE},
  {"|=",NONE},{"<", NONE},{">", NONE},{"<=",NONE},
  {">=",NONE},{"=", NONE},{"!=",NONE},{".",DOTOP}
};

Map_Str_U8 kwds = {
  {"break",   2},{"do",      3},{"echo", 4},
  {"else",    5},{"for",     6},{"if",   7},
  {"is",      9},{"match",  10},{"new", 11},
  {"next",   12},{"raw",    15},{"redo",16},
  {"return", 17},{"then",   18},{"use", 19},
  {"__exit",128},{"__line",129}
};

#endif

typedef struct Expr {
  Expr* sup;
  Vec(Expr*) sub;
  Value val;
  Type type;
  Oper oper;
} Expr;
Expr* expr;

char* outpath = "out";
bool run = true;

enum State {
  LINE, COML, SYMB, BSPC,
  OPER, ZERO, BINN, OCTN,
  DECN, HEXN, QUOT
};
State state;

char c;
Int16 lncnt;
char* buf;

void eval_sym (void)
{    
  switch (kwds[buf_sym]) {
    case 20   : run = false;
                break;
    case 23   : println(line_cnt);
                break;
    default   : break;
  }
  buf_sym.clear();
  switch (c) {
    case ' '  : state = BSPC;
                break;
    case_OPR  : buf_opr.push(chr);
                state = OPER;
                break;
    default   : break; 
  }
}

void eval_opr (void)
{    
  if (oprs[buf_opr]) {
    switch (expr->type) {
      case STRING:
      case NUMBER:
        expr = new Object {0,{expr},0,EXPR,
          oprs[buf_opr],preces[oprs[buf_opr]]};
        expr->objs[0]->sup = expr;
        break;
      case EXPR  :
        expr = new Object {0,{expr},0,EXPR,
          oprs[buf_opr],preces[oprs[buf_opr]]};
        expr->objs[0]->sup = expr;
        break;
    }
  }
  buf_opr.clear();
  switch (c) {
    case ' '  : state = BSPC;
                break;
    case_LOW  : buf_sym.push_back(chr);
                state = SYMB;
                break;
    default   : break;
  }
}

long long eval_exp (Object*& obj) {
  long long result = 0;
  switch (obj->type) {
    case EXPR: switch (obj->oper) {
      case PLUS:
        result = eval_exp(obj->objs[0]) + 
        eval_exp(obj->objs[1]); break;
      case MINUS:
        result = eval_exp(obj->objs[0]) - 
        eval_exp(obj->objs[1]); break;
      case MULT:
        result = eval_exp(obj->objs[0]) * 
        eval_exp(obj->objs[1]); break;
      case DIVD:
        result = eval_exp(obj->objs[0]) / 
        eval_exp(obj->objs[1]); break;
      case DOTOP:
        result = eval_exp(obj->objs[0]) +
        eval_exp(obj->objs[1]); break;
    } delete obj->objs[0]; delete obj->objs[1]; 
      obj->objs.shrink_to_fit(); break;
    case NUMBER: result = obj->value; break;
    case STRING: cout << *(string*)obj->str; break;
  }
  obj = 0;
  return result;
}

void eval_num (void)
{
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
  }
  buf_num.clear();
  switch (chr) {
    case ' ' : state = BSPC; break;
    default  : break;
  } if (!expr) expr = new Object { 0,{},value,NUMBER,NONE,PNONE };
  else if (expr->type == EXPR) switch (expr->oper) {
    case PLUS:
    case MINUS:
    case MULT:
    case DIVD:
      expr->objs.push_back( new Object
        { expr,{},value,NUMBER,NONE,PNONE } );
      break;  
  }
}

void eval_str (void)
{
  if (!expr) {
    expr = new Object {
      0,{},(long long)new string(buf_str),STRING,NONE,PNONE
    };
  }
  else if (expr->type == EXPR) {
    switch (expr->oper) {
      case DOTOP:
        expr->objs.push_back( new Object
          { expr,{},0,STRING,NONE,PNONE } );
        expr->objs[1]->str = new string(buf_str);
        break;
    }
  }
  buf_str.clear();
  state = BSPC;
}

void parse_line( Str* line )
{ 
  state = LINE;
  lncnt += 1;
  Int16 l = slen( line );
  Int16 i = 0;
  Int16 j = 0;

  while ( i < l ) {
    c = lile[i];

    switch (state) {

      case LINE   : switch (c) {
        case ' '  : ++j; goto next;
        case '#'  : j = 0;
                    state = COML;
                    goto next;
        case '\"' : state = QUOT;
                    goto next;
        case '0'  : state = ZERO;
                    goto next;
        case_NUM  : j = 1;
                    state = DECN;
                    goto next;
        case '_'  :
        case_LOW  : j = 1;
                    state = SYMB;
                    goto next;
      }
      
      case COML   : goto next;

      case SYMB   : switch (c) {
        default   : eval_sym();
                    goto next;
        case '_'  :
        case_LOW  : ++j;
                    goto next;
      }
      
      case BSPC   : switch (c) {
        case ' '  : goto next;
        case '\"' : state = QUOT;
                    goto next;
        case_OPR  : ++j;
                    state = OPER;
                    goto next;
        case_LOW  : ++j;
                    state = SYMB;
                    goto next;
        case_NUM  : ++j;
                    state = DECN;
                    goto next;
      }

      case OPER   : switch (c) {
        default   : eval_opr();
                    goto next;
        case_OPR  : ++j;
                    goto next;
      }

      case ZERO   : switch (c) {
        case 'b'  : state = BINN;
                    goto next;
        case 'o'  : state = OCTN;
                    goto next;
        case 'h'  : state = HEXN;
                    goto next;
      }

      case BINN   : switch (c) {
        default   : eval_num();
                    goto next;
        case_BIN  : ++j;
                    goto next;
      }

      case OCTN   : switch (c) {
        default   : eval_num(c);
                    goto next;
        case_OCT  : ++j;
                    goto next;
      }

      case DECN   : switch (c) {
        default   : eval_num();
                    goto next;
        case_DEC  : ++j;
                    goto next;
      } 

      case HEXN   : switch (c) {
        default   : eval_num();
                    goto next;
        case_HEX  : ++j;
                    goto next;
      }

      case QUOT   : switch (c) {
        case '\"' : eval_str();
                    goto next;
        default   : ++j;
                    goto next;
      }

      default     : goto next;
    }

  next: ++i;
  }

  switch (state) {
    case LINE :
    case COML :
    case BSPC : break;
    case SYMB : eval_sym('\n');
                break;
    case BINN :
    case OCTN :
    case DECN :
    case HEXN : eval_num('\n');
                break;
  }

  if (expr) {
    println(eval_exp(expr));
  }
}

char**
parse_args (long argc, char** argv)
{
  bool f_usg = false;
  bool f_ver = false;
  bool tmp = false;
  char** files = vnew();  

  for (
    long i = 1;
    i < argc;
    ++i )
  {
    if (argv[i][0] == '-') {
      switch (args[argv[i]]) {
        case 1  : f_usg = true;
                  break;
        case 2  : f_ver = true;
                  break;
        case 3  : tmp = true;
                  break;
        default : break;
      }    
    }
    else if (tmp) {
      outpath = argv[i];
      tmp = false;
    }
    else {
      vpush(files,argv[i]);
    }
  }

  if (f_usg) {
    print(USG);
    _exit(0);
  }

  if (f_ver) {
    print(VER);
    _exit(0);
  }

  return files;
}


 
void
 (void)
{
  char* line = snew();
  char** paths = parse_args(argc, argv);
  Int16 l = vlen(paths);
  if (l > 0) {
    for (int i = 0; i < l; ++i) {
      println(paths[i]);
    }
    exit(1);
  }
  else {
    while (run) {
      sprint(prompt_prefix);
      sscan(STDIN, line);
      parse_line(line);
    }
  }
  exit(0);
}

