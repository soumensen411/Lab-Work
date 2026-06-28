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

// ---------------- PARSE TREE ----------------
struct Node {
    string val;
    vector<Node*> children;
    Node(string v) { val = v; }
};

Token current() { return tokens[pos]; }
void advance() { if (pos < tokens.size()) pos++; }

// Expression parsing
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

    // Read files
    string code = readFile(cFile);
    string cfgText = readFile(cfgFile);

    // Store CFG
    auto grammar = readCFG(cfgText);

    cout << "\n===== CFG =====\n";
    for (auto &g : grammar) {
        for (auto &r : g.second)
            cout << g.first << " -> " << r << endl;
    }

    // Step 1: Remove comments
    string clean = removeComments(code);
    cout << "\n===== CLEAN CODE =====\n" << clean << endl;

    // Step 2: Tokenize
    tokens = tokenize(clean);

    cout << "\n===== TOKENS =====\n";
    for (auto &t : tokens) {
        cout << "<";
        if (t.type == KEYWORD) cout << "KEYWORD";
        else if (t.type == IDENTIFIER) cout << "IDENTIFIER";
        else if (t.type == NUMBER) cout << "NUMBER";
        else if (t.type == OPERATOR) cout << "OPERATOR";
        else cout << "SYMBOL";
        cout << ", " << t.value << ">\n";
    }

    // Step 3: Parse Tree
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