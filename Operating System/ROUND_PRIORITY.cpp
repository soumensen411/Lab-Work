#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n, tq;
    cin >> n;

    vector<int> at(n), bt(n), pr(n), rt(n), ct(n);
    vector<int> ready(n, 0), order;

    for (int i = 0; i < n; i++)
    {
        cin >> at[i] >> bt[i] >> pr[i];
        rt[i] = bt[i];
    }

    cin >> tq;

    int time = 0, finished = 0;
    int cur = -1, quantum = 0;

    while (finished < n)
    {
        if (cur != -1 && (rt[cur] == 0 || quantum == tq))
        {
            if (rt[cur] > 0)
                ready[cur] = time * 2 + 1;

            cur = -1;
        }

        if (cur != -1)
        {
            for (int i = 0; i < n; i++)
            {
                if (rt[i] > 0 && at[i] <= time &&
                    pr[i] < pr[cur])
                {
                    ready[cur] = time * 2 + 1;
                    cur = -1;
                    break;
                }
            }
        }

        if (cur == -1)
        {
            int best = -1;

            for (int i = 0; i < n; i++)
            {
                if (rt[i] > 0 && at[i] <= time)
                {
                    if (best == -1 ||
                        pr[i] < pr[best] ||
                        (pr[i] == pr[best] &&
                         ready[i] < ready[best]))
                    {
                        best = i;
                    }
                }
            }

            if (best == -1)
            {
                if (order.empty() || order.back() != 0)
                    order.push_back(0);

                time++;
                continue;
            }

            cur = best;
            quantum = 0;
        }

        if (order.empty() || order.back() != cur + 1)
            order.push_back(cur + 1);

        rt[cur]--;
        time++;
        quantum++;

        if (rt[cur] == 0)
        {
            ct[cur] = time;
            finished++;
        }
    }

    cout << "\nExecution Order: ";

    for (int i = 0; i < order.size(); i++)
    {
        if (order[i] == 0)
            cout << "Idle";
        else
            cout << "P" << order[i];

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