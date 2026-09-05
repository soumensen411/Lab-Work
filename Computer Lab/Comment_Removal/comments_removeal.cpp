#include <bits/stdc++.h>
using namespace std;

int main()
{
    ifstream fin("input.c");
    ofstream fout("output.c");

    if (!fin.is_open() || !fout.is_open())
    {
        cout << "Error opening file";
        return 1;
    }

    char c;
    enum State {NORMAL,IN_STRING,IN_CHAR,IN_SL_COMMENT,IN_ML_COMMENT};
    State state = NORMAL;

    while(fin.get(c)){
        if (state == NORMAL){
            if (c == '/'){
                if(fin.peek() == '/'){
                    state = IN_SL_COMMENT;
                    fin.get(c);
                    continue;
                }
                else if(fin.peek() == '*'){
                    state = IN_ML_COMMENT;
                    fin.get(c);
                    continue;
                }
                else{
                    fout.put(c);
                }
            }
            else if ( c == '"'){
                state = IN_STRING;
                fout.put(c);
            }
            else if (c == '\''){
                state = IN_CHAR;
                fout.put(c);
            }
            else{
                fout.put(c);
            }
        }
        else if (state == IN_STRING){
            fout.put(c);
            if(c == '\\'){
                if(fin.get(c)) fout.put(c);
            }
            else if(c == '"'){
                state = NORMAL;
            }
        }
        else if (state == IN_CHAR){
            fout.put(c);
            if(c == '\\'){
                if(fin.get(c))fout.put(c);
            }
            else if(c == '\''){
                state = NORMAL;
            }
        }
        else if(state == IN_SL_COMMENT){
            if(c == '\n'){
                fout.put('\n');
                state = NORMAL;
            }
        }
        else if(state == IN_ML_COMMENT){
            if (c == '*' && fin.peek() == '/'){
                fin.get(c);
                state = NORMAL;
            }
        }
    }
    fin.close();
    fout.close();
    cout<<"successfull";
}