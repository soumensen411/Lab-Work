#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n, tq;
    cin >> n;

    vector<int> at(n), bt(n), rt(n), ct(n);
    vector<int> order;
    vector<bool> inQ(n, false);
    queue<int> q;

    for (int i = 0; i < n; i++)
    {
        cin >> at[i] >> bt[i];
        rt[i] = bt[i];
    }

    cin >> tq;

    int time = 0, completed = 0;

    while (completed < n)
    {
        for (int i = 0; i < n; i++)
        {
            if (at[i] <= time && !inQ[i] && rt[i] > 0)
            {
                q.push(i);
                inQ[i] = true;
            }
        }

        if (q.empty())
        {
            time++;
            continue;
        }

        int idx = q.front();
        q.pop();

        order.push_back(idx);

        int run = min(tq, rt[idx]);
        time += run;
        rt[idx] -= run;

        for (int i = 0; i < n; i++)
        {
            if (at[i] <= time && !inQ[i] && rt[i] > 0)
            {
                q.push(i);
                inQ[i] = true;
            }
        }

        if (rt[idx] > 0)
        {
            q.push(idx);
        }
        else
        {
            ct[idx] = time;
            completed++;
        }
    }

    cout << "\nExecution Order: ";
    for (int i = 0; i < order.size(); i++)
    {
        cout << "P" << order[i] + 1;
        if (i != order.size() - 1)
            cout << " -> ";
    }

    double sumTAT = 0, sumWT = 0;

    for (int i = 0; i < n; i++)
    {
        int tat = ct[i] - at[i];
        int wt = tat - bt[i];

        sumTAT += tat;
        sumWT += wt;
    }

    cout << fixed << setprecision(2);
    cout << "\nAverage TAT = " << sumTAT / n;
    cout << "\nAverage WT = " << sumWT / n << endl;

    return 0;
}