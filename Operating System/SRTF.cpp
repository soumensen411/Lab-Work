

#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    vector<int> at(n), bt(n), rt(n), ct(n), tat(n), wt(n);
    vector<bool> done(n, false);

    for (int i = 0; i < n; i++)
    {
        cin >> at[i] >> bt[i];
        rt[i] = bt[i];
    }

    int time = 0, completed = 0;
    vector<int> process;

    while (completed < n)
    {
        int idx = -1;

        for (int i = 0; i < n; i++)
        {
            if (!done[i] && at[i] <= time && rt[i] > 0)
            {
                if (idx == -1 || rt[i] < rt[idx])
                    idx = i;
            }
        }

        if (idx == -1)
        {
            process.push_back(-1); // CPU Idle
            time++;
            continue;
        }

        process.push_back(idx); // Store every execution step

        rt[idx]--;
        time++;

        if (rt[idx] == 0)
        {
            ct[idx] = time;
            tat[idx] = ct[idx] - at[idx];
            wt[idx] = tat[idx] - bt[idx];

            done[idx] = true;
            completed++;
        }
    }

    cout << "Execution Order: ";
    for (int i : process)
    {
        if (i == -1)
            cout << "Idle ";
        else
            cout << "P" << i + 1 << " ";
    }
    cout << endl;

    double tatsum = 0, wtsum = 0;

    for (int i = 0; i < n; i++)
    {
        tatsum += tat[i];
        wtsum += wt[i];
    }

    cout << fixed << setprecision(2);
    cout << "AVG TAT TIME = " << tatsum / n << endl;
    cout << "AVG WT TIME = " << wtsum / n << endl;

    return 0;
}
