#include <bits/stdc++.h>
using namespace std;

// ---------------- TOKEN ----------------
enum TokenType { KEYWORD, IDENTIFIER, NUMBER, OPERATOR, SYMBOL };

struct Token {
    TokenType type;
    string value;
};

vector<Token> tokens;
int pos = 0;

// ---------------- FILE READ ----------------
string readFile(string filename) {
    ifstream file(filename);
    if (!file) {
        cout << "Error opening file: " << filename << endl;
        exit(1);
    }
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ---------------- COMMENT REMOVER ----------------
string removeComments(string code) {
    string result;
    bool single = false, multi = false;

    for (int i = 0; i < code.size(); i++) {
        if (!single && !multi && i+1 < code.size() && code[i]=='/' && code[i+1]=='/')
            single = true, i++;
        else if (!single && !multi && i+1 < code.size() && code[i]=='/' && code[i+1]=='*')
            multi = true, i++;
        else if (single && code[i]=='\n')
            single = false;
        else if (multi && i+1 < code.size() && code[i]=='*' && code[i+1]=='/')
            multi = false, i++;
        else if (!single && !multi)
            result += code[i];
    }
    return result;
}

// ---------------- TOKENIZER ----------------
vector<Token> tokenize(string code) {
    vector<Token> tks;
    set<string> keywords = {"int"};

    for (int i = 0; i < code.size();) {

        if (isspace(code[i])) i++;

        else if (isalpha(code[i])) {
            string word;
            while (isalnum(code[i])) word += code[i++];
            if (keywords.count(word)) tks.push_back({KEYWORD, word});
            else tks.push_back({IDENTIFIER, word});
        }

        else if (isdigit(code[i])) {
            string num;
            while (isdigit(code[i])) num += code[i++];
            tks.push_back({NUMBER, num});
        }

        else if (string("+-*/=").find(code[i]) != string::npos) {
            tks.push_back({OPERATOR, string(1, code[i])});
            i++;
        }

        else {
            tks.push_back({SYMBOL, string(1, code[i])});
            i++;
        }
    }
    return tks;
}

// ---------------- CFG STORAGE ----------------
map<string, vector<string>> readCFG(string cfgText) {
    map<string, vector<string>> grammar;
    stringstream ss(cfgText);
    string line;

    while (getline(ss, line)) {
        if (line.empty()) continue;

        int pos = line.find("->");
        if (pos == string::npos) continue;

        string left = line.substr(0, pos);
        string right = line.substr(pos + 2);

        left.erase(remove_if(left.begin(), left.end(), ::isspace), left.end());

        grammar[left].push_back(right);
    }

    return grammar;
}

// ---------------- REMOVE LEFT RECURSION ----------------
map<string, vector<string>> removeLeftRecursion(map<string, vector<string>> grammar) {
    map<string, vector<string>> newGrammar;

    for (auto &rule : grammar) {
        string A = rule.first;
        vector<string> alpha, beta;

        for (auto &prod : rule.second) {
            stringstream ss(prod);
            string first;
            ss >> first;

            if (first == A) {
                string rest = prod.substr(prod.find(first) + first.length());
                alpha.push_back(rest);
            } else {
                beta.push_back(prod);
            }
        }

        if (!alpha.empty()) {
            string A_dash = A + "'";

            for (auto &b : beta)
                newGrammar[A].push_back(b + " " + A_dash);

            for (auto &a : alpha)
                newGrammar[A_dash].push_back(a + " " + A_dash);

            newGrammar[A_dash].push_back("ε");
        }
        else {
            newGrammar[A] = rule.second;
        }
    }

    return newGrammar;
}

// ---------------- PARSE TREE ----------------
struct Node {
    string val;
    vector<Node*> children;
    Node(string v) { val = v; }
};

Token current() { return tokens[pos]; }
void advance() { if (pos < tokens.size()) pos++; }

Node* parseE();

Node* parseFactor() {
    if (current().type == IDENTIFIER || current().type == NUMBER) {
        Node* node = new Node(current().value);
        advance();
        return node;
    }
    return nullptr;
}

Node* parseTerm() {
    Node* left = parseFactor();

    while (pos < tokens.size() &&
          (current().value == "*" || current().value == "/")) {

        string op = current().value;
        advance();

        Node* right = parseFactor();
        Node* opNode = new Node(op);
        opNode->children.push_back(left);
        opNode->children.push_back(right);

        left = opNode;
    }
    return left;
}

Node* parseE() {
    Node* left = parseTerm();

    while (pos < tokens.size() &&
          (current().value == "+" || current().value == "-")) {

        string op = current().value;
        advance();

        Node* right = parseTerm();
        Node* opNode = new Node(op);
        opNode->children.push_back(left);
        opNode->children.push_back(right);

        left = opNode;
    }
    return left;
}

Node* parseAssignment() {
    if (current().type != IDENTIFIER) return nullptr;

    Node* root = new Node("=");
    root->children.push_back(new Node(current().value));
    advance();

    if (current().value != "=") return nullptr;
    advance();

    Node* expr = parseE();
    root->children.push_back(expr);

    return root;
}

// ---------------- PRINT TREE ----------------
void printTree(Node* root, int depth = 0) {
    if (!root) return;
    for (int i = 0; i < depth; i++) cout << "  ";
    cout << root->val << endl;
    for (auto child : root->children)
        printTree(child, depth + 1);
}

// ---------------- MAIN ----------------
int main() {

    string cFile, cfgFile;

    cout << "Enter C file: ";
    cin >> cFile;

    cout << "Enter CFG file: ";
    cin >> cfgFile;

    string code = readFile(cFile);
    string cfgText = readFile(cfgFile);

    // CFG
    auto grammar = readCFG(cfgText);

    cout << "\n===== ORIGINAL CFG =====\n";
    for (auto &g : grammar)
        for (auto &r : g.second)
            cout << g.first << " -> " << r << endl;

    auto newGrammar = removeLeftRecursion(grammar);

    cout << "\n===== CFG AFTER LEFT RECURSION REMOVAL =====\n";
    for (auto &g : newGrammar)
        for (auto &r : g.second)
            cout << g.first << " -> " << r << endl;

    // Code processing
    string clean = removeComments(code);

    cout << "\n===== CLEAN CODE =====\n" << clean << endl;

    tokens = tokenize(clean);

    cout << "\n===== TOKENS =====\n";
    for (auto &t : tokens) {
        cout << "<";
        if (t.type == KEYWORD) cout << "KEYWORD";
        else if (t.type == IDENTIFIER) cout << "ID";
        else if (t.type == NUMBER) cout << "NUM";
        else if (t.type == OPERATOR) cout << "OP";
        else cout << "SYM";
        cout << ", " << t.value << ">\n";
    }

    // Parse Tree
    cout << "\n===== PARSE TREE =====\n";
    pos = 0;

    while (pos < tokens.size()) {
        if (tokens[pos].type == IDENTIFIER) {
            Node* tree = parseAssignment();
            printTree(tree);
        }
        pos++;
    }

    return 0;
}
