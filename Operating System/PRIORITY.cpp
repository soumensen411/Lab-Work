
#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    int at[20], bt[20], pr[20];
    int ct[20], tat[20], wt[20];
    bool done[20] = {false};
    int time = 0, completed = 0;

    for (int i = 0; i < n; i++)
    {
        cin >> at[i] >> bt[i] >> pr[i];
    }

    while (completed < n)
    {
        int idx = -1;

        for (int i = 0; i < n; i++)
        {
            if (!done[i] && at[i] <= time)
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
            continue;
        }

        cout << "P" << idx + 1 << " ";

        time += bt[idx];
        ct[idx] = time;
        tat[idx] = ct[idx] - at[idx];
        wt[idx] = tat[idx] - bt[idx];

        done[idx] = true;
        completed++;
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
