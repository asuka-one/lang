//g++ main.cpp -o main
 
/*

x y
x >> y, x -> y
x[y]
+x, -x, ~x, ?x, !x, $x
x..y, x...y
new x
x & y
x ~ y
x | y
x * y, x / y
x + y, x - y
x . y
x :- y, x := y, x += y, x -= y, x *= y, x /= y,
  x &= y, x ~= y, x |= y, x .= y
x < y, x > y, x <= x, x >= y
x = y, x != y
not x
x and y
x or y

*/

/*

$
| |=
& &=
! !=
?
~ ~ ~=
=
* *=
/ /=
+ + +=
- - -= ->
: :- :=
. .. .= ...
< <=
>> > >=

*/

#include <unordered_map>
#include <iostream>
#include <vector>
#include <utility>
#include <string>

using namespace std;

#define case_LOW \
  case 'a': case 'b': case 'c': \
  case 'd': case 'e': case 'f': \
  case 'g': case 'h': case 'i': \
  case 'j': case 'k': case 'l': \
  case 'm': case 'n': case 'o': \
  case 'p': case 'q': case 'r': \
  case 's': case 't': case 'u': \
  case 'v': case 'w': case 'x': \
  case 'y': case 'z'
#define case_CAP \
  case 'A': case 'B': case 'C': \
  case 'D': case 'E': case 'F': \
  case 'G': case 'H': case 'I': \
  case 'J': case 'K': case 'L': \
  case 'M': case 'N': case 'O': \
  case 'P': case 'Q': case 'R': \
  case 'S': case 'T': case 'U': \
  case 'V': case 'W': case 'X': \
  case 'Y': case 'Z'
#define case_BIN \
  case '0': case '1'
#define case_OCT \
  case_BIN: case '2': case '3': \
  case '4': case '5': case '6': \
  case '7'
#define case_DEC \
  case_OCT: case '8': case '9'
#define case_HEX \
  case_DEC: case 'A': case 'B': \
  case 'C': case 'D': case 'E': \
  case 'F'

string outpath = "out";
bool run = true;
enum State {
  LINE, COML, COMB, COMA, COMP,
  COMQ, PARL, SYMB, EXPR, BSPC
};
vector<State> state = {};
int line_cnt = 0, expr_nest = 0,
  comm_nest = 0, indt_nest = 0;
vector<int> indnt = {0};
string buf_str = "", buf_sym = "",
  buf_num = "", buf_spc = ""; 

void parse_line(string line) {
  
  auto eval_sym = [&](char chr) {
    static unordered_map<string,int> kwds = {
      {"and",    1},{"break",   2},
      {"do",     3},{"echo",    4},
      {"else",   5},{"for",     6},
      {"if",     7},{"in",      8},
      {"is",     9},{"matches",10},
      {"new",   11},{"next",   12},
      {"not",   13},{"or",     14},
      {"raw",   15},{"redo",   16},
      {"return",17},{"then",   18},
      {"use",   19},{"__exit", 20}
    };
    switch (kwds[buf_sym]) {
      case 20: run = false; break;
      default: break;
    }
    buf_sym.clear(); state.pop_back();
    switch (chr) {
      case ' ' : state.push_back(BSPC); break;
      default  : break; 
    }
  };

  auto eval_indnt = [&](char chr) {
    buf_spc.clear(); state.pop_back();
    switch (chr) {
      case '#' : state.push_back(COML); break;
      case '_' :
      case_LOW : buf_sym.push_back(chr); 
        state.push_back(SYMB); break;
      case '(' : state.push_back(PARL);
    }
  };
  
  state.push_back(LINE); line_cnt += 1;

  for (char chr : line) {
    switch (state.back()) {

      case LINE: switch (chr) {
        case ' ' : buf_spc.push_back(chr); break;
        default  : eval_indnt(chr);
      } break;
      
      case COML: break;

      case COMB: switch (chr) {
        case '#' : state.back() = COMA; break;
        case '(' : state.back() = COMP; break;
        case '\"': state.push_back(COMQ); break;
        default  : break;
      } break;

      case COMA: switch (chr) {
        case ')' : state.pop_back(); break;
        default  : state.back() = COMB;
      } break;

      case COMP: switch (chr) {
        case '#' : state.push_back(COMB); break;
        default  : state.back() = COMB;
      } break;

      case COMQ: switch (chr) {
        case '\"': state.pop_back(); break;
        default  : break;
      } break;

      case PARL: switch (chr) {
        case '#' : state.back() = COMB; break;
        default  : expr_nest += 1; state.push_back(EXPR);
      } break;

      case SYMB: switch (chr) {
        case ' ' : eval_sym(chr); break;
        case '_' :
        case_LOW : buf_sym.push_back(chr); break;
      } break;
      
      case BSPC: break;

      default  : break;
    }
    cout << "(" << chr << " " << state.back() << ") ";
  }
  cout << "\n";

  switch (state.back()) {
    case COML: state.pop_back(); break;
    case COMB: break;
    case COMA: state.back() = COMB; break;
    case COMP: state.back() = COMB; break;
    case COMQ: state.pop_back(); break;
    case BSPC: state.pop_back(); break;
    case SYMB: eval_sym('\n'); break;
  }
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
    vector<string> contents = {};
    
    // for (auto& path : paths) with open(path,"r") as file: contents += [file.read().splitlines()]
    // for content in contents: for line in content: parse_line(line)
  } else {
    while (run) { cout << "> "; getline(cin,line); parse_line(line); }
    // if (errors.count()) cout << errors << endl; else cout << "ok" << endl;
  }
  return 0;
}