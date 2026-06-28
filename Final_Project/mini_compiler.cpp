/*
 * ============================================================
 *   Mini C Compiler - Single File Implementation
 *   Phases: Comment Removal -> Tokenizer -> Syntax Analysis
 *           -> Three-Address Code -> Optimization -> Assembly
 * ============================================================
 *   Usage:  g++ -o compiler compiler.cpp
 *           ./compiler input.c cfg.txt
 * ============================================================
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iomanip>

using namespace std;

// -------------------------------------------------------------
// ANSI Colors for pretty terminal output
// -------------------------------------------------------------
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define BLUE    "\033[34m"

void printHeader(const string& title) {
    cout << "\n" << BOLD << CYAN
         << "+----------------------------------------------+\n"
         << "|  " << left << setw(44) << title << "|\n"
         << "+----------------------------------------------+"
         << RESET << "\n\n";
}

void printOk(const string& msg)    { cout << GREEN  << "  [OK] " << RESET << msg << "\n"; }
void printErr(const string& msg)   { cout << RED    << "  [!!] " << RESET << msg << "\n"; }
void printInfo(const string& msg)  { cout << YELLOW << "  [>>] " << RESET << msg << "\n"; }

// -------------------------------------------------------------
// TOKEN TYPES
// -------------------------------------------------------------
enum TokenType {
    TOK_KEYWORD, TOK_IDENTIFIER, TOK_NUMBER, TOK_STRING,
    TOK_OPERATOR, TOK_PUNCTUATION, TOK_UNKNOWN, TOK_EOF
};

string tokenTypeName(TokenType t) {
    switch(t) {
        case TOK_KEYWORD:     return "KEYWORD";
        case TOK_IDENTIFIER:  return "IDENTIFIER";
        case TOK_NUMBER:      return "NUMBER";
        case TOK_STRING:      return "STRING";
        case TOK_OPERATOR:    return "OPERATOR";
        case TOK_PUNCTUATION: return "PUNCTUATION";
        case TOK_UNKNOWN:     return "UNKNOWN";
        default:              return "EOF";
    }
}

struct Token {
    TokenType   type;
    string      value;
    int         line;
};

// -------------------------------------------------------------
// PHASE 1 - COMMENT REMOVAL
// -------------------------------------------------------------
string removeComments(const string& src) {
    string out;
    int i = 0, n = src.size();
    while (i < n) {
        // Single-line comment //
        if (i+1 < n && src[i]=='/' && src[i+1]=='/') {
            while (i < n && src[i] != '\n') i++;
        }
        // Multi-line comment /* */
        else if (i+1 < n && src[i]=='/' && src[i+1]=='*') {
            i += 2;
            while (i+1 < n && !(src[i]=='*' && src[i+1]=='/')) {
                if (src[i]=='\n') out += '\n';
                i++;
            }
            i += 2;
        }
        // String literal - don't strip inside
        else if (src[i]=='"') {
            out += src[i++];
            while (i < n && src[i] != '"') {
                if (src[i]=='\\') out += src[i++];
                out += src[i++];
            }
            if (i < n) out += src[i++];
        }
        else {
            out += src[i++];
        }
    }
    return out;
}

// -------------------------------------------------------------
// PHASE 2 - TOKENIZER
// -------------------------------------------------------------
static const set<string> KEYWORDS = {
    "int","float","double","char","void","if","else","while",
    "for","do","return","break","continue","switch","case",
    "default","struct","typedef","include","define","printf","scanf"
};

static const set<string> MULTI_OPS = {
    "==","!=","<=",">=","&&","||","++","--","+=","-=","*=","/="
};

vector<Token> tokenize(const string& src) {
    vector<Token> tokens;
    int i = 0, n = src.size(), line = 1;

    while (i < n) {
        // Skip whitespace
        if (isspace(src[i])) {
            if (src[i] == '\n') line++;
            i++; continue;
        }
        // String literal
        if (src[i] == '"') {
            string val = "\"";
            i++;
            while (i < n && src[i] != '"') {
                if (src[i]=='\\' && i+1<n) { val += src[i++]; }
                val += src[i++];
            }
            val += '"'; i++;
            tokens.push_back({TOK_STRING, val, line});
            continue;
        }
        // Char literal
        if (src[i] == '\'') {
            string val = "'";
            i++;
            while (i < n && src[i] != '\'') {
                if (src[i]=='\\' && i+1<n) val += src[i++];
                val += src[i++];
            }
            val += '\''; i++;
            tokens.push_back({TOK_STRING, val, line});
            continue;
        }
        // Number
        if (isdigit(src[i]) || (src[i]=='.' && i+1<n && isdigit(src[i+1]))) {
            string val;
            while (i < n && (isalnum(src[i]) || src[i]=='.')) val += src[i++];
            tokens.push_back({TOK_NUMBER, val, line});
            continue;
        }
        // Identifier / Keyword
        if (isalpha(src[i]) || src[i]=='_') {
            string val;
            while (i < n && (isalnum(src[i]) || src[i]=='_')) val += src[i++];
            TokenType t = KEYWORDS.count(val) ? TOK_KEYWORD : TOK_IDENTIFIER;
            tokens.push_back({t, val, line});
            continue;
        }
        // Preprocessor directive
        if (src[i] == '#') {
            string val = "#";
            i++;
            while (i < n && src[i] != '\n') val += src[i++];
            tokens.push_back({TOK_KEYWORD, val, line});
            continue;
        }
        // Multi-char operator
        if (i+1 < n) {
            string two = string(1,src[i]) + src[i+1];
            if (MULTI_OPS.count(two)) {
                tokens.push_back({TOK_OPERATOR, two, line});
                i += 2; continue;
            }
        }
        // Single-char operator / punctuation
        char c = src[i++];
        string cs(1,c);
        if (string("+-*/%=<>&|!~^").find(c) != string::npos)
            tokens.push_back({TOK_OPERATOR,    cs, line});
        else if (string("(){};:,[]").find(c) != string::npos)
            tokens.push_back({TOK_PUNCTUATION, cs, line});
        else
            tokens.push_back({TOK_UNKNOWN,     cs, line});
    }
    tokens.push_back({TOK_EOF, "EOF", line});
    return tokens;
}

// -------------------------------------------------------------
// PHASE 3 - SYNTAX ANALYSIS (using CFG from cfg.txt)
// -------------------------------------------------------------
// trim leading/trailing whitespace
static string trimStr(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Split string by delimiter, return trimmed non-empty parts
static vector<string> splitBy(const string& s, char delim) {
    vector<string> parts;
    istringstream ss(s);
    string tok;
    while (getline(ss, tok, delim)) {
        string t = trimStr(tok);
        if (!t.empty()) parts.push_back(t);
    }
    return parts;
}

// Parse one alternative "E + T" into symbols ["E", "+", "T"]
static vector<string> parseAlt(const string& alt) {
    vector<string> syms;
    istringstream ss(alt);
    string s;
    while (ss >> s) syms.push_back(s);
    return syms;
}

// One rule groups all alternatives for the same LHS
// e.g.  E -> E + T | E - T | T
struct CFGRule {
    string lhs;
    vector<vector<string>> alternatives;
};

// Load CFG from file.
// Supports:  S -> E
//            E -> E + T | E - T | T
//             T -> id | num
// Blank lines and lines starting with '#' are ignored.
vector<CFGRule> loadCFG(const string& path) {
    vector<CFGRule> rules;
    map<string,int> lhsIdx;

    ifstream f(path);
    if (!f.is_open()) return rules;
    string line;
    while (getline(f, line)) {
        string tl = trimStr(line);
        if (tl.empty() || tl[0] == '#') continue;
        size_t arrow = tl.find("->");
        if (arrow == string::npos) continue;

        string lhs     = trimStr(tl.substr(0, arrow));
        string rhs_all = tl.substr(arrow + 2);

        // Split by '|' to get alternatives
        vector<string> altStrs = splitBy(rhs_all, '|');

        // Find or create entry for this LHS
        if (!lhsIdx.count(lhs)) {
            lhsIdx[lhs] = (int)rules.size();
            CFGRule r; r.lhs = lhs;
            rules.push_back(r);
        }
        CFGRule& rule = rules[lhsIdx[lhs]];
        for (auto& altStr : altStrs) {
            vector<string> syms = parseAlt(altStr);
            if (!syms.empty()) rule.alternatives.push_back(syms);
        }
    }
    return rules;
}

struct SyntaxError { int line; string msg; };

// Simple recursive-descent style checker for C subset
// Returns errors found in token stream
vector<SyntaxError> syntaxAnalysis(const vector<Token>& tokens) {
    vector<SyntaxError> errors;
    int i = 0, n = tokens.size();

    // helpers
    auto peek = [&]() -> const Token& { return tokens[min(i,(int)n-1)]; };
    auto consume = [&]() -> const Token& {
        const Token& t = tokens[min(i,(int)n-1)];
        if (i < n) i++;
        return t;
    };

    // Check balanced braces/parens/brackets
    vector<pair<char,int>> stack;
    map<char,char> closing = {{')','{'},{'}',' '},{']','['}};
    map<char,char> match   = {{'(',')'},{'{','}'},{'[',']'}};

    for (auto& tok : tokens) {
        if (tok.type == TOK_EOF) break;
        char c = tok.value[0];
        if (c=='(' || c=='{' || c=='[') {
            stack.push_back({c, tok.line});
        } else if (c==')' || c=='}' || c==']') {
            if (stack.empty()) {
                errors.push_back({tok.line, "Unexpected '" + tok.value + "' - no matching opener"});
            } else if (match[stack.back().first] != c) {
                errors.push_back({tok.line, "Mismatched bracket: expected '" +
                    string(1,match[stack.back().first]) + "' but got '" + tok.value + "'"});
                stack.pop_back();
            } else {
                stack.pop_back();
            }
        }
    }
    for (auto& p : stack) {
        errors.push_back({p.second, string("Unclosed '") + p.first + "'"});
    }

    // Check every statement ends with ; (very simplified)
    // Look for patterns: keyword(if/while/for) must be followed by (
    for (int j = 0; j < (int)tokens.size()-1; j++) {
        auto& t = tokens[j];
        if (t.type==TOK_KEYWORD && (t.value=="if"||t.value=="while"||t.value=="for")) {
            if (tokens[j+1].value != "(") {
                errors.push_back({t.line, "'" + t.value + "' must be followed by '('"});
            }
        }
        // return without ;
        if (t.type==TOK_KEYWORD && t.value=="return") {
            // look ahead for ;
            int k = j+1;
            bool found = false;
            while (k < (int)tokens.size() && tokens[k].value != "\n") {
                if (tokens[k].value == ";") { found = true; break; }
                if (tokens[k].value == "}" || tokens[k].type==TOK_EOF) break;
                k++;
            }
            if (!found && tokens[k].value != "}") {
                // might be void return
            }
        }
    }

    // Check for consecutive operators (e.g., a ++ + b is ok, but a ++ ++ is suspicious)
    for (int j = 0; j+1 < (int)tokens.size(); j++) {
        if (tokens[j].type==TOK_OPERATOR && tokens[j+1].type==TOK_OPERATOR) {
            string a = tokens[j].value, b = tokens[j+1].value;
            // Allow unary minus/plus after operator
            set<string> unary = {"-","+","!","~","*","&"};
            set<string> incDec= {"++","--"};
            if (!incDec.count(a) && !unary.count(b) && !incDec.count(b)) {
                errors.push_back({tokens[j].line,
                    "Suspicious consecutive operators: '" + a + "' '" + b + "'"});
            }
        }
    }

    return errors;
}

// -------------------------------------------------------------
// PHASE 4 - THREE-ADDRESS CODE (TAC) GENERATION
// -------------------------------------------------------------
struct TACInstr {
    string result, op, arg1, arg2;
};

static int tmpCount = 0;
static int lblCount = 0;
string newTemp()  { return "t" + to_string(++tmpCount); }
string newLabel() { return "L" + to_string(++lblCount); }

// Build TAC from tokens (simplified expression + control flow)
vector<TACInstr> generateTAC(const vector<Token>& tokens) {
    vector<TACInstr> code;
    int i = 0, n = tokens.size();

    auto peek = [&]() -> const Token& { return tokens[min(i,(int)n-1)]; };
    auto consume = [&]() { if(i<n) i++; };
    auto match = [&](const string& v) -> bool {
        if (peek().value == v) { consume(); return true; } return false;
    };

    // Extremely simplified: scan for assignments like: var = expr ;
    // and if/while blocks, and return statements
    while (i < n && peek().type != TOK_EOF) {
        Token t = peek();

        // Declaration + possible assignment:  int x = expr ;
        if (t.type==TOK_KEYWORD && (t.value=="int"||t.value=="float"||t.value=="char"||t.value=="double")) {
            consume(); // type
            if (peek().type == TOK_IDENTIFIER) {
                string varName = peek().value; consume();
                if (peek().value == "=") {
                    consume(); // =
                    // collect RHS until ;
                    string rhs;
                    while (peek().value != ";" && peek().type != TOK_EOF) {
                        rhs += peek().value; consume();
                    }
                    if (peek().value==";") consume();
                    // simple: if rhs is "a op b"
                    // try to split
                    string ops = "+-*/";
                    string found_op = "";
                    string a1, a2;
                    for (char op : ops) {
                        size_t pos = rhs.find(op);
                        if (pos != string::npos && pos > 0) {
                            found_op = string(1,op);
                            a1 = rhs.substr(0,pos);
                            a2 = rhs.substr(pos+1);
                            break;
                        }
                    }
                    if (!found_op.empty())
                        code.push_back({varName, found_op, a1, a2});
                    else
                        code.push_back({varName, "=", rhs, ""});
                } else if (peek().value==";") consume();
                else consume();
            } else consume();
            continue;
        }

        // Assignment: identifier = expr ;
        if (t.type==TOK_IDENTIFIER && i+1<n && tokens[i+1].value=="=") {
            string varName = t.value; consume(); consume(); // id =
            string rhs;
            while (peek().value != ";" && peek().type != TOK_EOF && peek().value != ")") {
                rhs += peek().value; consume();
            }
            if (peek().value==";") consume();
            string ops = "+-*/";
            string found_op = "", a1, a2;
            for (char op : ops) {
                size_t pos = rhs.find(op);
                if (pos!=string::npos && pos>0) {
                    found_op = string(1,op);
                    a1 = rhs.substr(0,pos);
                    a2 = rhs.substr(pos+1);
                    break;
                }
            }
            if (!found_op.empty()) {
                string tmp = newTemp();
                code.push_back({tmp, found_op, a1, a2});
                code.push_back({varName, "=", tmp, ""});
            } else {
                code.push_back({varName, "=", rhs, ""});
            }
            continue;
        }

        // if ( cond ) { body }
        if (t.type==TOK_KEYWORD && t.value=="if") {
            consume();
            string cond;
            if (peek().value=="(") { consume(); }
            while (peek().value != ")" && peek().type != TOK_EOF)
                { cond += peek().value; consume(); }
            if (peek().value==")") consume();
            string lbl_else = newLabel(), lbl_end = newLabel();
            code.push_back({"", "ifFalse " + cond + " goto", lbl_else, ""});
            // skip body for now
            continue;
        }

        // while ( cond )
        if (t.type==TOK_KEYWORD && t.value=="while") {
            consume();
            string cond;
            string lbl_start = newLabel(), lbl_end = newLabel();
            code.push_back({"", "label", lbl_start, ""});
            if (peek().value=="(") consume();
            while (peek().value != ")" && peek().type != TOK_EOF)
                { cond += peek().value; consume(); }
            if (peek().value==")") consume();
            code.push_back({"", "ifFalse " + cond + " goto", lbl_end, ""});
            code.push_back({"", "goto", lbl_start, ""});
            code.push_back({"", "label", lbl_end, ""});
            continue;
        }

        // return expr ;
        if (t.type==TOK_KEYWORD && t.value=="return") {
            consume();
            string val;
            while (peek().value != ";" && peek().type != TOK_EOF)
                { val += peek().value; consume(); }
            if (peek().value==";") consume();
            code.push_back({"", "return", val, ""});
            continue;
        }

        // printf / scanf
        if (t.type==TOK_KEYWORD && (t.value=="printf"||t.value=="scanf")) {
            string fn = t.value; consume();
            string args;
            if (peek().value=="(") consume();
            int depth=1;
            while (depth>0 && peek().type!=TOK_EOF) {
                if (peek().value=="(") depth++;
                if (peek().value==")") { depth--; if(depth==0){consume();break;} }
                args += peek().value; consume();
            }
            if (peek().value==";") consume();
            code.push_back({"", "call "+fn, args, ""});
            continue;
        }

        consume(); // skip unknown tokens
    }
    return code;
}

// -------------------------------------------------------------
// PHASE 5 - CODE OPTIMIZATION
// -------------------------------------------------------------
vector<TACInstr> optimize(vector<TACInstr> code) {
    // Pass 1: Constant Folding
    for (auto& ins : code) {
        if (!ins.arg2.empty()) {
            bool isNum1 = !ins.arg1.empty() && all_of(ins.arg1.begin(), ins.arg1.end(),
                [](char c){ return isdigit(c)||c=='.'||c=='-'; });
            bool isNum2 = !ins.arg2.empty() && all_of(ins.arg2.begin(), ins.arg2.end(),
                [](char c){ return isdigit(c)||c=='.'||c=='-'; });
            if (isNum1 && isNum2) {
                double a = stod(ins.arg1), b = stod(ins.arg2), r = 0;
                bool folded = true;
                if      (ins.op=="+") r = a+b;
                else if (ins.op=="-") r = a-b;
                else if (ins.op=="*") r = a*b;
                else if (ins.op=="/" && b!=0) r = a/b;
                else folded = false;
                if (folded) {
                    // Remove decimal if whole number
                    ostringstream oss;
                    if (r == (long long)r) oss << (long long)r;
                    else oss << r;
                    ins.op  = "=";
                    ins.arg1 = oss.str();
                    ins.arg2 = "";
                    printInfo("  Constant folding: " + ins.result + " = " + ins.arg1);
                }
            }
        }
    }

    // Pass 2: Dead Code Elimination (remove t = x; then x never used)
    set<string> used;
    for (auto& ins : code) {
        if (!ins.arg1.empty() && ins.arg1[0]!='L') used.insert(ins.arg1);
        if (!ins.arg2.empty() && ins.arg2[0]!='L') used.insert(ins.arg2);
    }
    vector<TACInstr> cleaned;
    for (auto& ins : code) {
        // Keep if result is used somewhere or it's a control/call instr
        if (ins.result.empty() || used.count(ins.result) ||
            ins.op.find("goto")!=string::npos ||
            ins.op.find("label")!=string::npos ||
            ins.op.find("call")!=string::npos ||
            ins.op.find("return")!=string::npos) {
            cleaned.push_back(ins);
        } else {
            printInfo("  Dead code removed: " + ins.result);
        }
    }

    // Pass 3: Copy Propagation  (x = y; z = x; -> z = y)
    map<string,string> copyMap;
    for (auto& ins : cleaned) {
        // propagate into args
        if (copyMap.count(ins.arg1)) ins.arg1 = copyMap[ins.arg1];
        if (copyMap.count(ins.arg2)) ins.arg2 = copyMap[ins.arg2];
        // record copy
        if (ins.op == "=" && !ins.arg1.empty() && ins.arg2.empty())
            copyMap[ins.result] = ins.arg1;
    }

    return cleaned;
}

// -------------------------------------------------------------
// PHASE 6 - CODE GENERATION (x86-like Assembly)
// -------------------------------------------------------------
string generateAssembly(const vector<TACInstr>& code) {
    ostringstream asm_;
    asm_ << "; -- x86-like Assembly Output ------------------\n";
    asm_ << "section .data\n";
    asm_ << "section .text\n";
    asm_ << "global _main\n_main:\n";
    asm_ << "  push ebp\n  mov  ebp, esp\n\n";

    auto isNum = [](const string& s) -> bool {
        if (s.empty()) return false;
        int i = 0;
        if (s[0]=='-') i=1;
        return all_of(s.begin()+i, s.end(), [](char c){ return isdigit(c)||c=='.'; });
    };
    auto reg = [](const string& s) -> string {
        // Map temp vars to EAX/EBX/ECX or use stack notation
        static map<string,string> regMap;
        static vector<string> regs = {"eax","ebx","ecx","edx","esi","edi"};
        if (isdigit(s[0]) || s[0]=='-') return s; // immediate
        if (!regMap.count(s)) {
            if (regMap.size() < regs.size())
                regMap[s] = regs[regMap.size()];
            else
                regMap[s] = "[ebp-" + to_string((regMap.size()-regs.size()+1)*4) + "]";
        }
        return regMap[s];
    };

    for (auto& ins : code) {
        if (ins.op == "label") {
            asm_ << ins.arg1 << ":\n";
        }
        else if (ins.op == "=") {
            string dst = reg(ins.result);
            string src = isNum(ins.arg1) ? ins.arg1 : reg(ins.arg1);
            asm_ << "  mov  " << dst << ", " << src << "\n";
        }
        else if (ins.op == "+" || ins.op == "-" || ins.op == "*" || ins.op == "/") {
            string dst  = reg(ins.result);
            string src1 = isNum(ins.arg1) ? ins.arg1 : reg(ins.arg1);
            string src2 = isNum(ins.arg2) ? ins.arg2 : reg(ins.arg2);
            asm_ << "  mov  " << dst << ", " << src1 << "\n";
            if      (ins.op=="+") asm_ << "  add  " << dst << ", " << src2 << "\n";
            else if (ins.op=="-") asm_ << "  sub  " << dst << ", " << src2 << "\n";
            else if (ins.op=="*") asm_ << "  imul " << dst << ", " << src2 << "\n";
            else if (ins.op=="/") {
                asm_ << "  mov  eax, " << src1 << "\n";
                asm_ << "  cdq\n";
                asm_ << "  idiv " << src2 << "\n";
                asm_ << "  mov  " << dst << ", eax\n";
            }
        }
        else if (ins.op.find("ifFalse") != string::npos) {
            // ifFalse cond goto Lx
            asm_ << "  ; ifFalse " << ins.arg1 << " -> " << ins.arg1 << "\n";
            asm_ << "  cmp  eax, 0\n";
            asm_ << "  je   " << ins.arg1 << "\n";
        }
        else if (ins.op == "goto") {
            asm_ << "  jmp  " << ins.arg1 << "\n";
        }
        else if (ins.op.find("return") != string::npos) {
            if (!ins.arg1.empty()) {
                asm_ << "  mov  eax, "
                     << (isNum(ins.arg1) ? ins.arg1 : reg(ins.arg1)) << "\n";
            }
            asm_ << "  pop  ebp\n  ret\n";
        }
        else if (ins.op.find("call") != string::npos) {
            // printf / scanf
            asm_ << "  ; " << ins.op << "(" << ins.arg1 << ")\n";
            asm_ << "  push " << ins.arg1 << "\n";
            string fn = ins.op.substr(5); // remove "call "
            asm_ << "  call " << fn << "\n";
            asm_ << "  add  esp, 4\n";
        }
        else {
            asm_ << "  ; " << ins.result << " " << ins.op
                 << " " << ins.arg1 << " " << ins.arg2 << "\n";
        }
    }

    asm_ << "\n  mov  eax, 0\n";
    asm_ << "  pop  ebp\n";
    asm_ << "  ret\n";
    asm_ << "; ------------------------------------------------\n";
    return asm_.str();
}

// -------------------------------------------------------------
//  PRETTY PRINT HELPERS
// -------------------------------------------------------------
void printTokenTable(const vector<Token>& tokens) {
    cout << BOLD << "  " << left
         << setw(6)  << "Line"
         << setw(18) << "Token Value"
         << setw(14) << "Type"
         << RESET << "\n";
    cout << "  " << string(38,'-') << "\n";
    for (auto& t : tokens) {
        if (t.type == TOK_EOF) break;
        string typeStr = tokenTypeName(t.type);
        string color = RESET;
        if (t.type==TOK_KEYWORD)     color = CYAN;
        if (t.type==TOK_NUMBER)      color = YELLOW;
        if (t.type==TOK_STRING)      color = GREEN;
        if (t.type==TOK_OPERATOR)    color = MAGENTA;
        if (t.type==TOK_IDENTIFIER)  color = BLUE;
        cout << "  " << left
             << setw(6)  << t.line
             << color << setw(18) << t.value << RESET
             << setw(14) << typeStr << "\n";
    }
}

void printTAC(const vector<TACInstr>& code) {
    int idx = 1;
    for (auto& ins : code) {
        cout << "  " << YELLOW << setw(3) << idx++ << RESET << "  ";
        if (ins.op == "label") {
            cout << CYAN << ins.arg1 << ":" << RESET;
        } else if (ins.op == "=" && ins.arg2.empty()) {
            cout << ins.result << " = " << ins.arg1;
        } else if (!ins.arg2.empty()) {
            cout << ins.result << " = " << ins.arg1 << " " << ins.op << " " << ins.arg2;
        } else if (ins.op.find("goto")!=string::npos || ins.op.find("ifFalse")!=string::npos) {
            cout << ins.op << " " << ins.arg1;
        } else if (ins.op.find("call")!=string::npos) {
            cout << ins.op << "(" << ins.arg1 << ")";
        } else if (ins.op.find("return")!=string::npos) {
            cout << "return " << ins.arg1;
        } else {
            cout << ins.result << " " << ins.op << " " << ins.arg1 << " " << ins.arg2;
        }
        cout << "\n";
    }
}

// -------------------------------------------------------------
//  MAIN
// -------------------------------------------------------------
int main(int argc, char* argv[]) {
    cout << BOLD << "\n"
         << "  +--------------------------------------------+\n"
         << "  |      Mini C Compiler  - All Phases        |\n"
         << "  +--------------------------------------------+\n"
         << RESET;

    if (argc < 3) {
        printErr("Usage: " + string(argv[0]) + " input.c cfg.txt");
        return 1;
    }

    string cFile  = argv[1];
    string cfgFile = argv[2];

    // -- Read input.c ------------------------------------------
    ifstream fin(cFile);
    if (!fin.is_open()) { printErr("Cannot open: " + cFile); return 1; }
    string source((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
    fin.close();
    printOk("Loaded source file: " + cFile);

    // -- Load CFG ---------------------------------------------
    vector<CFGRule> cfg = loadCFG(cfgFile);
    int totalProds = 0;
    for (auto& r : cfg) totalProds += (int)r.alternatives.size();
    printOk("Loaded CFG: " + cfgFile + " (" + to_string(cfg.size()) +
            " non-terminals, " + to_string(totalProds) + " productions)");
    if (!cfg.empty()) {
        cout << "\n  " << BOLD << "CFG Rules:" << RESET << "\n";
        for (auto& r : cfg) {
            // Print:  E  ->  E + T | E - T | T
            cout << "    " << CYAN << setw(14) << left << r.lhs << RESET << " ->  ";
            for (int ai = 0; ai < (int)r.alternatives.size(); ai++) {
                if (ai > 0) cout << YELLOW << " | " << RESET;
                for (auto& sym : r.alternatives[ai]) cout << sym << " ";
            }
            cout << "\n";
        }
        cout << "\n";
    }

    // ========================================================
    // PHASE 1 - Comment Removal
    // ========================================================
    printHeader("Phase 1 - Comment Removal");
    string clean = removeComments(source);
    printOk("Comments stripped successfully.");
    cout << "\n  " << BOLD << "Clean Source:" << RESET << "\n";
    istringstream ss(clean);
    string ln; int lno=1;
    while(getline(ss,ln)) {
        if(!ln.empty())
            cout << "  " << YELLOW << setw(3) << lno << RESET << "  " << ln << "\n";
        lno++;
    }

    // ========================================================
    // PHASE 2 - Tokenization
    // ========================================================
    printHeader("Phase 2 - Tokenization");
    vector<Token> tokens = tokenize(clean);
    printOk("Total tokens: " + to_string(tokens.size()-1));
    cout << "\n";
    printTokenTable(tokens);

    // ========================================================
    // PHASE 3 - Syntax Analysis
    // ========================================================
    printHeader("Phase 3 - Syntax Analysis");
    vector<SyntaxError> errors = syntaxAnalysis(tokens);
    if (errors.empty()) {
        printOk("Syntax analysis PASSED - No errors found.");
    } else {
        printErr("Syntax analysis found " + to_string(errors.size()) + " error(s):");
        for (auto& e : errors)
            cout << RED << "    Line " << e.line << ": " << e.msg << RESET << "\n";
        cout << "\n" << YELLOW << "  Attempting to continue with best-effort compilation...\n" << RESET;
    }

    // ========================================================
    // PHASE 4 - TAC Generation
    // ========================================================
    printHeader("Phase 4 - Three-Address Code Generation");
    vector<TACInstr> tac = generateTAC(tokens);
    if (tac.empty()) {
        printInfo("No TAC instructions generated (source may be declarations only).");
    } else {
        printOk("TAC instructions generated: " + to_string(tac.size()));
        cout << "\n";
        printTAC(tac);
    }

    // ========================================================
    // PHASE 5 - Optimization
    // ========================================================
    printHeader("Phase 5 - Code Optimization");
    vector<TACInstr> optTAC = optimize(tac);
    int removed = (int)tac.size() - (int)optTAC.size();
    printOk("Optimization complete. Instructions reduced by " + to_string(removed) + ".");
    cout << "\n  " << BOLD << "Optimized TAC:" << RESET << "\n";
    if (optTAC.empty()) printInfo("  (empty - nothing to optimize)");
    else printTAC(optTAC);

    // ========================================================
    // PHASE 6 - Assembly Code Generation
    // ========================================================
    printHeader("Phase 6 - Assembly Code Generation");
    string asmCode = generateAssembly(optTAC);
    printOk("Assembly generated successfully.");
    cout << "\n" << GREEN;
    istringstream asmStream(asmCode);
    string asmLine;
    while (getline(asmStream, asmLine))
        cout << "    " << asmLine << "\n";
    cout << RESET;

    // Write assembly to output file
    string outFile = "output.asm";
    ofstream fout(outFile);
    fout << asmCode;
    fout.close();
    printOk("Assembly written to: " + outFile);

    // -- Final Summary -----------------------------------------
    cout << BOLD << "\n"
         << "  +--------------------------------------------+\n"
         << "  |             Compilation Summary           |\n"
         << "  +--------------------------------------------+\n"
         << "  |  Tokens      : " << GREEN << setw(28) << tokens.size()-1 << RESET << BOLD << "|\n"
         << "  |  Syntax Errs : " << (errors.empty()?GREEN:RED) << setw(28) << errors.size() << RESET << BOLD << "|\n"
         << "  |  TAC Instrs  : " << YELLOW << setw(28) << tac.size()    << RESET << BOLD << "|\n"
         << "  |  After Opt.  : " << CYAN   << setw(28) << optTAC.size() << RESET << BOLD << "|\n"
         << "  |  Output      : " << GREEN  << setw(28) << outFile        << RESET << BOLD << "|\n"
         << "  +--------------------------------------------+\n"
         << RESET << "\n";

    return errors.empty() ? 0 : 1;
}