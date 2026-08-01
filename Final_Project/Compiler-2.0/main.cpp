#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

const string EPSILON = "#";   // symbol we use for "empty string"
const string END_MARK = "$";  // symbol we use for "end of input"

/* ==================================================================
   Small helper functions used everywhere below
   ================================================================== */

// Split a string like "E + T" into words: ["E", "+", "T"]
vector<string> splitWords(string text) {
    vector<string> words;
    stringstream stream(text);
    string w;
    while (stream >> w) words.push_back(w);
    return words;
}

// Join words back into one string with spaces: ["E","+","T"] -> "E + T"
string joinWords(vector<string> words) {
    string result = "";
    for (int i = 0; i < (int)words.size(); i++) {
        result += words[i];
        if (i + 1 < (int)words.size()) result += " ";
    }
    return result;
}

// Add a string to a vector only if it is not already there
void addIfNew(vector<string> &v, string item) {
    for (int i = 0; i < (int)v.size(); i++) {
        if (v[i] == item) return; // already present
    }
    v.push_back(item);
}

// Check whether "item" exists inside vector v
bool contains(vector<string> &v, string item) {
    for (int i = 0; i < (int)v.size(); i++) {
        if (v[i] == item) return true;
    }
    return false;
}

/* ==================================================================
   STAGE 1: COMMENT REMOVAL
   ================================================================== */
string removeComments(string src) {
    string result = "";
    int i = 0;
    int n = (int)src.size();

    while (i < n) {
        // Copy string literals ("...") exactly as they are
        if (src[i] == '"') {
            result += src[i];
            i++;
            while (i < n && src[i] != '"') {
                if (src[i] == '\\' && i + 1 < n) { // escaped character like \"
                    result += src[i];
                    result += src[i + 1];
                    i += 2;
                } else {
                    result += src[i];
                    i++;
                }
            }
            if (i < n) { result += src[i]; i++; } // closing quote
            continue;
        }

        // Copy char literals ('x') exactly as they are
        if (src[i] == '\'') {
            result += src[i];
            i++;
            while (i < n && src[i] != '\'') {
                if (src[i] == '\\' && i + 1 < n) {
                    result += src[i];
                    result += src[i + 1];
                    i += 2;
                } else {
                    result += src[i];
                    i++;
                }
            }
            if (i < n) { result += src[i]; i++; }
            continue;
        }

        // Line comment: //......  -> skip until newline
        if (src[i] == '/' && i + 1 < n && src[i + 1] == '/') {
            while (i < n && src[i] != '\n') i++;
            continue;
        }

        // Block comment: /* ... */ -> skip everything inside
        if (src[i] == '/' && i + 1 < n && src[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(src[i] == '*' && src[i + 1] == '/')) i++;
            i += 2;
            continue;
        }

        // Normal character, just keep it
        result += src[i];
        i++;
    }
    return result;
}

/* ==================================================================
   STAGE 2: TOKENIZATION
   ================================================================== */
struct Token {
    string kind;   // "keyword", "identifier", "number", "operator", "symbol", "string", "char", "preprocessor"
    string text;
};

vector<string> KEYWORDS = {
    "int", "float", "double", "char", "void", "long", "short", "unsigned",
    "return", "if", "else", "while", "for", "do", "switch", "case",
    "break", "continue", "struct", "typedef", "const", "static", "sizeof"
};

vector<string> TWO_CHAR_OPERATORS = {
    "==", "!=", "<=", ">=", "&&", "||", "++", "--", "+=", "-=", "*=", "/="
};

vector<string> ONE_CHAR_OPERATORS = {
    "+", "-", "*", "/", "%", "=", "<", ">", "!", "&", "|", "^"
};

vector<Token> tokenize(string code) {
    vector<Token> tokens;
    int i = 0;
    int n = (int)code.size();

    while (i < n) {
        char c = code[i];

        // Skip spaces, tabs, newlines
        if (isspace((unsigned char)c)) { i++; continue; }

        // Preprocessor line, like #include <stdio.h>
        if (c == '#') {
            int start = i;
            while (i < n && code[i] != '\n') i++;
            Token t; t.kind = "preprocessor"; t.text = code.substr(start, i - start);
            tokens.push_back(t);
            continue;
        }

        // String literal
        if (c == '"') {
            int start = i;
            i++;
            while (i < n && code[i] != '"') {
                if (code[i] == '\\') i++;
                i++;
            }
            if (i < n) i++;
            Token t; t.kind = "string"; t.text = code.substr(start, i - start);
            tokens.push_back(t);
            continue;
        }

        // Char literal
        if (c == '\'') {
            int start = i;
            i++;
            while (i < n && code[i] != '\'') {
                if (code[i] == '\\') i++;
                i++;
            }
            if (i < n) i++;
            Token t; t.kind = "char"; t.text = code.substr(start, i - start);
            tokens.push_back(t);
            continue;
        }

        // Identifier or keyword
        if (isalpha((unsigned char)c) || c == '_') {
            int start = i;
            while (i < n && (isalnum((unsigned char)code[i]) || code[i] == '_')) i++;
            string word = code.substr(start, i - start);
            Token t;
            t.text = word;
            t.kind = contains(KEYWORDS, word) ? "keyword" : "identifier";
            tokens.push_back(t);
            continue;
        }

        // Number
        if (isdigit((unsigned char)c)) {
            int start = i;
            while (i < n && (isdigit((unsigned char)code[i]) || code[i] == '.')) i++;
            Token t; t.kind = "number"; t.text = code.substr(start, i - start);
            tokens.push_back(t);
            continue;
        }

        // Two-character operator, e.g. ==, <=, ++
        if (i + 1 < n) {
            string two = code.substr(i, 2);
            if (contains(TWO_CHAR_OPERATORS, two)) {
                Token t; t.kind = "operator"; t.text = two;
                tokens.push_back(t);
                i += 2;
                continue;
            }
        }

        // One-character operator
        string one(1, c);
        if (contains(ONE_CHAR_OPERATORS, one)) {
            Token t; t.kind = "operator"; t.text = one;
            tokens.push_back(t);
            i++;
            continue;
        }

        // Symbols like ; { } ( ) ,
        if (string(";{}(),[]").find(c) != string::npos) {
            Token t; t.kind = "symbol"; t.text = one;
            tokens.push_back(t);
            i++;
            continue;
        }

        // Unknown character, just skip it
        i++;
    }
    return tokens;
}

// Print all tokens of one kind, without repeating the same word twice
void printOneRow(string label, vector<Token> &tokens, string kind) {
    vector<string> seen;
    cout << left;
    cout.width(20); cout << label;
    for (int i = 0; i < (int)tokens.size(); i++) {
        if (tokens[i].kind == kind && !contains(seen, tokens[i].text)) {
            seen.push_back(tokens[i].text);
            cout << tokens[i].text << "  ";
        }
    }
    cout << "\n";
}

void printTokenTable(vector<Token> &tokens) {
    cout << left;
    cout.width(20); cout << "Token Type" << "Tokens\n";
    cout << string(70, '-') << "\n";
    printOneRow("Keywords", tokens, "keyword");
    printOneRow("Identifiers", tokens, "identifier");
    printOneRow("Numeric Constants", tokens, "number");
    printOneRow("Operators", tokens, "operator");
    printOneRow("Symbols", tokens, "symbol");
    printOneRow("Strings", tokens, "string");
    printOneRow("Char Literals", tokens, "char");
}

/* ==================================================================
   STAGE 3: GRAMMAR + LEFT RECURSION ELIMINATION
   ------------------------------------------------------------------
   A grammar is stored very simply:
     nonTerminals  -> list of non-terminal names, in order
     productions   -> map from a non-terminal name to a list of
                      right-hand sides (each right-hand side is one
                      string, e.g. "E + T")
   ================================================================== */

bool isNonTerminal(string symbol, vector<string> &nonTerminals) {
    return contains(nonTerminals, symbol);
}

void printGrammar(vector<string> &nonTerminals, map<string, vector<string> > &productions) {
    for (int i = 0; i < (int)nonTerminals.size(); i++) {
        string A = nonTerminals[i];
        cout << "  " << A << " -> ";
        vector<string> rhsList = productions[A];
        for (int j = 0; j < (int)rhsList.size(); j++) {
            cout << rhsList[j];
            if (j + 1 < (int)rhsList.size()) cout << " | ";
        }
        cout << "\n";
    }
}

// Removes DIRECT left recursion from every non-terminal.
// Rule:  A -> A a1 | A a2 | ... | b1 | b2 ...
// becomes:
//        A  -> b1 A' | b2 A' | ...
//        A' -> a1 A' | a2 A' | epsilon
void eliminateLeftRecursion(vector<string> &nonTerminals, map<string, vector<string> > &productions) {
    vector<string> newOrder;
    map<string, vector<string> > newProductions;

    for (int i = 0; i < (int)nonTerminals.size(); i++) {
        string A = nonTerminals[i];
        vector<string> recursivePart;     // the "a" parts (without leading A)
        vector<string> nonRecursivePart;  // the "b" parts

        vector<string> rhsList = productions[A];
        for (int j = 0; j < (int)rhsList.size(); j++) {
            vector<string> words = splitWords(rhsList[j]);
            if (words.size() > 0 && words[0] == A) {
                // starts with A itself -> left recursive, remove the leading A
                vector<string> alpha;
                for (int k = 1; k < (int)words.size(); k++) alpha.push_back(words[k]);
                recursivePart.push_back(joinWords(alpha));
            } else {
                nonRecursivePart.push_back(rhsList[j]);
            }
        }

        if (recursivePart.empty()) {
            // no left recursion here, keep production unchanged
            newOrder.push_back(A);
            newProductions[A] = rhsList;
            continue;
        }

        string Aprime = A + "'";
        vector<string> newA, newAprime;

        for (int j = 0; j < (int)nonRecursivePart.size(); j++) {
            string beta = nonRecursivePart[j];
            if (beta == EPSILON) newA.push_back(Aprime);
            else newA.push_back(beta + " " + Aprime);
        }
        for (int j = 0; j < (int)recursivePart.size(); j++) {
            string alpha = recursivePart[j];
            if (alpha == "") newAprime.push_back(Aprime); // just in case
            else newAprime.push_back(alpha + " " + Aprime);
        }
        newAprime.push_back(EPSILON);

        newOrder.push_back(A);
        newProductions[A] = newA;
        newOrder.push_back(Aprime);
        newProductions[Aprime] = newAprime;
    }

    nonTerminals = newOrder;
    productions = newProductions;
}

/* ==================================================================
   STAGE 4 & 5: FIRST and FOLLOW SETS
   ================================================================== */

// Computes FIRST of a sequence of symbols (like "T + F"), starting
// from position "start". Returns the symbols found in "outSymbols"
// and tells you whether the whole sequence can produce epsilon.
bool firstOfSequence(vector<string> words, int start,
                      map<string, vector<string> > &FIRST,
                      vector<string> &nonTerminals,
                      vector<string> &outSymbols) {
    bool nullable = true;
    for (int i = start; i < (int)words.size(); i++) {
        string sym = words[i];
        if (sym == EPSILON) continue;

        if (!isNonTerminal(sym, nonTerminals)) {
            addIfNew(outSymbols, sym);
            nullable = false;
            break;
        }

        vector<string> firstOfSym = FIRST[sym];
        bool hasEpsilon = false;
        for (int k = 0; k < (int)firstOfSym.size(); k++) {
            if (firstOfSym[k] == EPSILON) hasEpsilon = true;
            else addIfNew(outSymbols, firstOfSym[k]);
        }
        if (!hasEpsilon) { nullable = false; break; }
    }
    return nullable;
}

map<string, vector<string> > computeFirstSets(vector<string> &nonTerminals, map<string, vector<string> > &productions) {
    map<string, vector<string> > FIRST;
    for (int i = 0; i < (int)nonTerminals.size(); i++) FIRST[nonTerminals[i]] = vector<string>();

    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < (int)nonTerminals.size(); i++) {
            string A = nonTerminals[i];
            vector<string> rhsList = productions[A];
            for (int j = 0; j < (int)rhsList.size(); j++) {
                if (rhsList[j] == EPSILON) {
                    if (!contains(FIRST[A], EPSILON)) { FIRST[A].push_back(EPSILON); changed = true; }
                    continue;
                }
                vector<string> words = splitWords(rhsList[j]);
                vector<string> found;
                bool nullable = firstOfSequence(words, 0, FIRST, nonTerminals, found);

                int before = (int)FIRST[A].size();
                for (int k = 0; k < (int)found.size(); k++) addIfNew(FIRST[A], found[k]);
                if (nullable) addIfNew(FIRST[A], EPSILON);
                if ((int)FIRST[A].size() != before) changed = true;
            }
        }
    }
    return FIRST;
}

map<string, vector<string> > computeFollowSets(vector<string> &nonTerminals, map<string, vector<string> > &productions,
                                                map<string, vector<string> > &FIRST, string startSymbol) {
    map<string, vector<string> > FOLLOW;
    for (int i = 0; i < (int)nonTerminals.size(); i++) FOLLOW[nonTerminals[i]] = vector<string>();
    FOLLOW[startSymbol].push_back(END_MARK);

    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < (int)nonTerminals.size(); i++) {
            string A = nonTerminals[i];
            vector<string> rhsList = productions[A];
            for (int j = 0; j < (int)rhsList.size(); j++) {
                vector<string> words = splitWords(rhsList[j]);
                for (int pos = 0; pos < (int)words.size(); pos++) {
                    string B = words[pos];
                    if (!isNonTerminal(B, nonTerminals)) continue;

                    vector<string> found;
                    bool nullable = firstOfSequence(words, pos + 1, FIRST, nonTerminals, found);

                    int before = (int)FOLLOW[B].size();
                    for (int k = 0; k < (int)found.size(); k++) addIfNew(FOLLOW[B], found[k]);
                    if (nullable) {
                        for (int k = 0; k < (int)FOLLOW[A].size(); k++) addIfNew(FOLLOW[B], FOLLOW[A][k]);
                    }
                    if ((int)FOLLOW[B].size() != before) changed = true;
                }
            }
        }
    }
    return FOLLOW;
}

void printSetTable(string name, vector<string> &nonTerminals, map<string, vector<string> > &SETS) {
    for (int i = 0; i < (int)nonTerminals.size(); i++) {
        string A = nonTerminals[i];
        cout << name << "(" << A << ") = { ";
        vector<string> s = SETS[A];
        for (int k = 0; k < (int)s.size(); k++) {
            cout << s[k];
            if (k + 1 < (int)s.size()) cout << ", ";
        }
        cout << " }\n";
    }
}

/* ==================================================================
   STAGE 6: LL(1) PARSING TABLE
   ------------------------------------------------------------------
   table[A][terminal] = the right-hand-side string to use
   ================================================================== */
map<string, map<string, string> > buildParsingTable(
    vector<string> &nonTerminals, map<string, vector<string> > &productions,
    map<string, vector<string> > &FIRST, map<string, vector<string> > &FOLLOW,
    vector<string> &conflicts)
{
    map<string, map<string, string> > table;

    for (int i = 0; i < (int)nonTerminals.size(); i++) {
        string A = nonTerminals[i];
        vector<string> rhsList = productions[A];
        for (int j = 0; j < (int)rhsList.size(); j++) {
            string rhs = rhsList[j];

            if (rhs == EPSILON) {
                vector<string> follow = FOLLOW[A];
                for (int k = 0; k < (int)follow.size(); k++) {
                    string b = follow[k];
                    if (table[A].count(b)) conflicts.push_back(A + " on " + b);
                    table[A][b] = rhs;
                }
                continue;
            }

            vector<string> words = splitWords(rhs);
            vector<string> found;
            bool nullable = firstOfSequence(words, 0, FIRST, nonTerminals, found);

            for (int k = 0; k < (int)found.size(); k++) {
                string a = found[k];
                if (table[A].count(a)) conflicts.push_back(A + " on " + a);
                table[A][a] = rhs;
            }
            if (nullable) {
                vector<string> follow = FOLLOW[A];
                for (int k = 0; k < (int)follow.size(); k++) {
                    string b = follow[k];
                    if (table[A].count(b)) conflicts.push_back(A + " on " + b);
                    table[A][b] = rhs;
                }
            }
        }
    }
    return table;
}

void printParsingTable(vector<string> &nonTerminals, map<string, map<string, string> > &table, vector<string> &terminals) {
    cout << left;
    cout.width(8); cout << " ";
    for (int i = 0; i < (int)terminals.size(); i++) { cout.width(14); cout << terminals[i]; }
    cout << "\n";

    for (int i = 0; i < (int)nonTerminals.size(); i++) {
        string A = nonTerminals[i];
        cout.width(8); cout << A;
        for (int j = 0; j < (int)terminals.size(); j++) {
            string t = terminals[j];
            string cell = "-";
            if (table[A].count(t)) cell = A + " -> " + table[A][t];
            cout.width(14); cout << cell;
        }
        cout << "\n";
    }
}

/* ==================================================================
   STAGE 7: CHECKING  (predictive parser with a visible stack trace)
   ================================================================== */
string stackToString(vector<string> &stack) {
    string s = "";
    for (int i = (int)stack.size() - 1; i >= 0; i--) s += stack[i];
    return s;
}
string remainingInputToString(vector<string> &input, int pos) {
    string s = "";
    for (int i = pos; i < (int)input.size(); i++) s += input[i];
    return s;
}

bool runPredictiveParse(vector<string> &nonTerminals, map<string, map<string, string> > &table,
                         string startSymbol, vector<string> inputTokens) {
    vector<string> input = inputTokens;
    input.push_back(END_MARK);

    vector<string> stack;
    stack.push_back(END_MARK);
    stack.push_back(startSymbol);

    int pos = 0;

    cout << left;
    cout.width(16); cout << "Stack";
    cout.width(20); cout << "Input";
    cout << "Output\n";
    cout << string(60, '-') << "\n";

    while (!stack.empty()) {
        string top = stack.back();
        string current = input[pos];

        if (top == END_MARK && current == END_MARK) {
            cout.width(16); cout << stackToString(stack);
            cout.width(20); cout << remainingInputToString(input, pos);
            cout << "Accepted\n";
            return true;
        }

        if (!isNonTerminal(top, nonTerminals)) {
            // top of stack is a terminal, it must match the current input symbol
            if (top == current) {
                cout.width(16); cout << stackToString(stack);
                cout.width(20); cout << remainingInputToString(input, pos);
                cout << ("Match " + top) << "\n";
                stack.pop_back();
                pos++;
            } else {
                cout.width(16); cout << stackToString(stack);
                cout.width(20); cout << remainingInputToString(input, pos);
                cout << "Error: mismatch\n";
                return false;
            }
        } else {
            // top of stack is a non-terminal, look up the parsing table
            if (!table[top].count(current)) {
                cout.width(16); cout << stackToString(stack);
                cout.width(20); cout << remainingInputToString(input, pos);
                cout << "Error: no rule\n";
                return false;
            }
            string rhs = table[top][current];
            cout.width(16); cout << stackToString(stack);
            cout.width(20); cout << remainingInputToString(input, pos);
            cout << (top + " -> " + rhs) << "\n";

            stack.pop_back();
            if (rhs != EPSILON) {
                vector<string> words = splitWords(rhs);
                for (int i = (int)words.size() - 1; i >= 0; i--) stack.push_back(words[i]);
            }
        }
    }
    return false;
}

/* ==================================================================
   STAGE 8: THREE ADDRESS CODE
   ------------------------------------------------------------------
   Simple two-pass approach (no tree structure needed):
     Pass 1: turn every identifier/number into its own temp variable
     Pass 2: combine all "*" first (left to right), then all "+"
   ================================================================== */

int tempCounter = 0;
string makeTemp() {
    tempCounter++;
    return "t" + to_string(tempCounter);
}

// Repeatedly combines the first occurrence of "opSymbol" in the list,
// e.g. list = [t1, "*", t2] becomes [t3] with TAC line "t3 = t1 * t2"
void reduceOperator(vector<string> &list, string opSymbol, vector<string> &tacLines) {
    bool foundOne = true;
    while (foundOne) {
        foundOne = false;
        for (int i = 0; i < (int)list.size(); i++) {
            if (list[i] == opSymbol) {
                string left = list[i - 1];
                string right = list[i + 1];
                string newTemp = makeTemp();
                tacLines.push_back(newTemp + " = " + left + " " + opSymbol + " " + right);

                // replace list[i-1], list[i], list[i+1] with newTemp
                vector<string> updated;
                for (int k = 0; k < i - 1; k++) updated.push_back(list[k]);
                updated.push_back(newTemp);
                for (int k = i + 2; k < (int)list.size(); k++) updated.push_back(list[k]);
                list = updated;

                foundOne = true;
                break; // start scanning again from the beginning
            }
        }
    }
}

vector<string> generateThreeAddressCode(vector<string> &exprAtoms, string &finalTempOut) {
    vector<string> tacLines;
    vector<string> list; // will hold temp names and operators, e.g. [t1, "+", t2, "*", t3]

    for (int i = 0; i < (int)exprAtoms.size(); i++) {
        string atom = exprAtoms[i];
        if (atom == "+" || atom == "*") {
            list.push_back(atom);
        } else {
            string t = makeTemp();
            tacLines.push_back(t + " = " + atom);
            list.push_back(t);
        }
    }

    reduceOperator(list, "*", tacLines); // multiplication has higher precedence
    reduceOperator(list, "+", tacLines);

    finalTempOut = list[0]; // only one item should remain
    return tacLines;
}

/* ==================================================================
   STAGE 9: ASSEMBLY CODE GENERATION
   ================================================================== */
int regCounter = 0;
string nextRegister() {
    regCounter++;
    return "R" + to_string(regCounter);
}

vector<string> generateAssembly(vector<string> &tacLines, string targetVariable) {
    vector<string> asmLines;
    string lastTemp = "";

    for (int i = 0; i < (int)tacLines.size(); i++) {
        string line = tacLines[i];
        int eqPos = line.find(" = ");
        string lhs = line.substr(0, eqPos);
        string rhs = line.substr(eqPos + 3);
        vector<string> parts = splitWords(rhs);
        lastTemp = lhs;

        if (parts.size() == 1) {
            // simple copy: tX = value
            string r1 = nextRegister();
            asmLines.push_back("MOVF " + parts[0] + ", " + r1);
            asmLines.push_back("MOVF " + r1 + ", " + lhs);
            asmLines.push_back("");
        } else {
            // binary operation: tX = A op B
            string r1 = nextRegister();
            string r2 = nextRegister();
            string r3 = nextRegister();
            string opName = "ADDF";
            if (parts[1] == "*") opName = "MULF";
            asmLines.push_back("MOVF " + parts[0] + ", " + r1);
            asmLines.push_back("MOVF " + parts[2] + ", " + r2);
            asmLines.push_back(opName + " " + r1 + ", " + r2 + ", " + r3);
            asmLines.push_back("MOVF " + r3 + ", " + lhs);
            asmLines.push_back("");
        }
    }

    // store final result into the target variable, e.g. z
    string finalReg = nextRegister();
    asmLines.push_back("MOVF " + lastTemp + ", " + finalReg);
    asmLines.push_back("MOVF " + finalReg + ", " + targetVariable);
    return asmLines;
}

/* ==================================================================
   MAIN DRIVER
   ================================================================== */
void printHeading(string title) {
    cout << "\n--------------- " << title << " ---------------\n\n";
}

int main(int argc, char** argv) {
    string path = "input.c";
    if (argc > 1) path = argv[1];

    ifstream in(path);
    if (!in) {
        cout << "Could not open file: " << path << "\n";
        return 1;
    }
    stringstream buffer;
    buffer << in.rdbuf();
    string sourceCode = buffer.str();

    /* ---------- Stage 1 ---------- */
    printHeading("Stage 1: Comment Removal");
    string cleanedCode = removeComments(sourceCode);
    cout << cleanedCode << "\n";

    ofstream out("output.c");
    out << cleanedCode;
    out.close();

    /* ---------- Stage 2 ---------- */
    printHeading("Stage 2: Tokenization");
    vector<Token> tokens = tokenize(cleanedCode);
    printTokenTable(tokens);

    /* ---------- Stage 3 ---------- */
    printHeading("Stage 3: Grammar & Left Recursion Elimination");

    vector<string> nonTerminals;
    map<string, vector<string> > productions;

    nonTerminals.push_back("E");
    productions["E"].push_back("E + T");
    productions["E"].push_back("T");

    nonTerminals.push_back("T");
    productions["T"].push_back("T * F");
    productions["T"].push_back("F");

    nonTerminals.push_back("F");
    productions["F"].push_back("ID");
    productions["F"].push_back("num");

    string startSymbol = "E";

    cout << "---- Original Grammar ----\n";
    printGrammar(nonTerminals, productions);

    eliminateLeftRecursion(nonTerminals, productions);

    cout << "\n---- Grammar After Left-Recursion Elimination ----\n";
    printGrammar(nonTerminals, productions);

    /* ---------- Stage 4 ---------- */
    printHeading("Stage 4: FIRST Sets");
    map<string, vector<string> > FIRST = computeFirstSets(nonTerminals, productions);
    printSetTable("FIRST", nonTerminals, FIRST);

    /* ---------- Stage 5 ---------- */
    printHeading("Stage 5: FOLLOW Sets");
    map<string, vector<string> > FOLLOW = computeFollowSets(nonTerminals, productions, FIRST, startSymbol);
    printSetTable("FOLLOW", nonTerminals, FOLLOW);

    /* ---------- Stage 6 ---------- */
    printHeading("Stage 6: LL(1) Parsing Table");
    vector<string> conflicts;
    map<string, map<string, string> > table = buildParsingTable(nonTerminals, productions, FIRST, FOLLOW, conflicts);

    vector<string> terminals;
    terminals.push_back("*");
    terminals.push_back("+");
    terminals.push_back("ID");
    terminals.push_back("num");
    terminals.push_back("$");
    printParsingTable(nonTerminals, table, terminals);

    if (!conflicts.empty()) {
        cout << "\n!! Conflicts found (grammar is not LL(1)):\n";
        for (int i = 0; i < (int)conflicts.size(); i++) cout << "  " << conflicts[i] << "\n";
    }

    /* ---------- Stage 7 ---------- */
    printHeading("Stage 7: Checking (Predictive Parse)");

    // Find the first assignment like "IDENT = <expression> ;" in the token list
    vector<string> exprTerminals; // grammar symbols: ID, num, +, *
    vector<string> exprAtoms;     // actual text: variable names / numbers / operators
    string targetVariable = "";

    for (int i = 0; i + 1 < (int)tokens.size(); i++) {
        if (tokens[i].kind == "identifier" && tokens[i + 1].kind == "operator" && tokens[i + 1].text == "=") {
            targetVariable = tokens[i].text;
            int j = i + 2;
            while (j < (int)tokens.size() && !(tokens[j].kind == "symbol" && tokens[j].text == ";")) {
                if (tokens[j].kind == "identifier") {
                    exprTerminals.push_back("ID");
                    exprAtoms.push_back(tokens[j].text);
                } else if (tokens[j].kind == "number") {
                    exprTerminals.push_back("num");
                    exprAtoms.push_back(tokens[j].text);
                } else if (tokens[j].kind == "operator" && (tokens[j].text == "+" || tokens[j].text == "*")) {
                    exprTerminals.push_back(tokens[j].text);
                    exprAtoms.push_back(tokens[j].text);
                }
                j++;
            }
            break;
        }
    }

    if (exprTerminals.empty()) {
        cout << "(no arithmetic assignment statement found to check)\n";
    } else {
        cout << "Checking: " << targetVariable << " = ";
        for (int i = 0; i < (int)exprAtoms.size(); i++) cout << exprAtoms[i] << " ";
        cout << "\n\n";

        bool accepted = runPredictiveParse(nonTerminals, table, startSymbol, exprTerminals);
        cout << "\n>> " << (accepted ? "Accepted." : "Rejected.") << "\n";

        /* ---------- Stage 8 ---------- */
        printHeading("Stage 8: Three Address Code");
        string finalTemp;
        vector<string> tacLines = generateThreeAddressCode(exprAtoms, finalTemp);
        for (int i = 0; i < (int)tacLines.size(); i++) cout << tacLines[i] << "\n";
        cout << targetVariable << " = " << finalTemp << "\n";

        /* ---------- Stage 9 ---------- */
        printHeading("Stage 9: Assembly Code");
        vector<string> asmLines = generateAssembly(tacLines, targetVariable);
        for (int i = 0; i < (int)asmLines.size(); i++) cout << asmLines[i] << "\n";
    }

    cout << "\n";
    return 0;
}