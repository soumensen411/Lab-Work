
#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    int at[20], bt[20], pr[20];
    int ct[20], tat[20], wt[20];
    int rem[20];

    for (int i = 0; i < n; i++)
    {
        cin >> at[i] >> bt[i] >> pr[i];
        rem[i] = bt[i];
    }

    int time = 0, completed = 0;
    int prev = -1;

    while (completed < n)
    {
        int idx = -1;

        for (int i = 0; i < n; i++)
        {
            if (at[i] <= time && rem[i] > 0)
            {
                if (idx == -1 || pr[i] < pr[idx])
                {
                    idx = i;
                }
            }
        }

        if (idx == -1)
        {
            time++;
            prev = -1;
            continue;
        }

        if (prev != idx)
        {
            cout << "P" << idx + 1 << " ";
        }

        rem[idx]--;
        time++;

        if (rem[idx] == 0)
        {
            ct[idx] = time;
            tat[idx] = ct[idx] - at[idx];
            wt[idx] = tat[idx] - bt[idx];
            completed++;
        }

        prev = idx;
    }

    double tatsum = 0, wtsum = 0;

    for (int i = 0; i < n; i++)
    {
        tatsum += tat[i];
        wtsum += wt[i];
    }

    cout << "\nAverage TAT = " << tatsum / n << endl;
    cout << "Average WT = " << wtsum / n << endl;

    return 0;
}
