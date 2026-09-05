#include <bits/stdc++.h>
using namespace std;

// token sets (no duplicate values)
set<string> kw, id, num, strVal, op, sym;

// keyword list
vector<string> kwList = {
    "int","float","char","double","void","if","else",
    "while","for","do","return","break","continue","struct"
};

// check keyword
bool isKeyword(string w) {
    return find(kwList.begin(), kwList.end(), w) != kwList.end();
}

// store word
void storeWord(string w) {
    if (w.empty()) return;

    if (isKeyword(w)) kw.insert(w);
    else if (isdigit(w[0])) num.insert(w);
    else if (isalpha(w[0]) || w[0] == '_') id.insert(w);
}

// scanner
void scanCode(string code) {
    int i = 0, n = code.size();

    while (i < n) {

        // skip space
        if (isspace(code[i])) {
            i++; continue;
        }

        // string / char
        if (code[i] == '"' || code[i] == '\'') {
            char q = code[i];
            string s = "";
            s += q; i++;

            while (i < n && code[i] != q) {
                s += code[i++];
            }

            s += q;
            strVal.insert(s);
            i++;
            continue;
        }

        // number
        if (isdigit(code[i])) {
            string s = "";
            while (i < n && (isdigit(code[i]) || code[i] == '.')) {
                s += code[i++];
            }
            num.insert(s);
            continue;
        }

        // identifier / keyword
        if (isalpha(code[i]) || code[i] == '_') {
            string s = "";
            while (i < n && (isalnum(code[i]) || code[i] == '_')) {
                s += code[i++];
            }
            storeWord(s);
            continue;
        }

        // double operator
        if (i + 1 < n) {
            string two = code.substr(i, 2);
            if (two=="==" || two=="!=" || two==">=" || two=="<=" ||
                two=="++" || two=="--" || two=="&&" || two=="||") {
                op.insert(two);
                i += 2;
                continue;
            }
        }

        // single operator
        if (string("+-*/%=<>!&|").find(code[i]) != string::npos) {
            op.insert(string(1, code[i]));
            i++;
            continue;
        }

        // symbol
        if (string(";,(){}[]").find(code[i]) != string::npos) {
            sym.insert(string(1, code[i]));
            i++;
            continue;
        }

        i++;
    }
}

// print function
void printSet(string name, set<string> s) {
    cout << "\n[" << name << "]\n";
    if (s.empty()) {
        cout << "  (none)\n";
        return;
    }
    for (auto x : s) {
        cout << "  " << x << endl;
    }
}

int main() {
    cout << "Enter code (type END to finish):\n\n";

    string code = "", line;
    while (getline(cin, line) && line != "END") {
        code += line + "\n";
    }

    scanCode(code);

    cout << "\n===== LEXICAL ANALYSIS =====\n";
    printSet("Keywords", kw);
    printSet("Identifiers", id);
    printSet("Numbers", num);
    printSet("Strings", strVal);
    printSet("Operators", op);
    printSet("Symbols", sym);

    return 0;
}
/*
int a = 10;
float b = 3.14;
char c = 'x';
char name[] = "hello";
if (a > 5) {
    a++;
}
else {
    a--;
}
while (a != 0) {
    a = a - 1;
}
return 0;
END
*/