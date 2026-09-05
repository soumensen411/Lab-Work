#include <bits/stdc++.h>
using namespace std;

bool identifier(const string &s)
{
    if (!(isalpha(s[0]) || s[0] == '_'))
        return false;
    for (int i = 1; i < s.size(); i++)
    {
        if (!(isalnum(s[i]) || s[i] == '_'))
        {
            return false;
        }
    }
    return true;
}

bool isNumber(const string &s)
{
    bool dotseen = false, expseen = false;
    int i = 0;
    if (s[i] == '+' || s[i] == '-')
    {
        i++;
    }
    for (; i < s.size(); i++)
    {
        if (isdigit(s[i]))
            continue;
        else if (s[i] == '.' && !dotseen)
            dotseen = true;
        else if ((s[i] == 'e' || s[i] == 'E') && !expseen)
        {
            expseen = true;
            if (i + 1 < s.size() && (s[i + 1] == '+' || s[i + 1] == '-'))
            {
                i++;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

int main()
{
    string token;
    cout << "Enter a token: ";
    cin >> token;

    if (identifier(token))
        cout << token << " is a valid Identifier\n";
    else if (isNumber(token))
        cout << token << " is a valid Number\n";
    else
        cout << token << " is Invalid\n";

    return 0;
}
/*
myVar
12.5
1ab
3e10
*/