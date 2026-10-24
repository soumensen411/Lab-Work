
#include <bits/stdc++.h>
using namespace std;

int main()
{
    int n;
    cin >> n;

    int at[n], bt[n], pr[n];
    int ct[n], tat[n], wt[n];
    int p[n];

    for (int i = 0; i < n; i++)
    {
        cin >> pr[i] >> bt[i] >> at[i];
        p[i] = i;
    }

    int time = 0;
    double tatsum = 0, wtsum = 0;

    for (int i = 0; i < n; i++)
    {
        int idx = -1;

        for (int j = 0; j < n; j++)
        {
            if (p[j] != -1 && at[j] <= time)
            {
                if (idx == -1 || pr[j] > pr[idx])
                    idx = j;
            }
        }

        if (idx == -1)
        {
            time++;
            i--;
            continue;
        }

        time += bt[idx];
        ct[idx] = time;

        tat[idx] = ct[idx] - at[idx];
        wt[idx] = tat[idx] - bt[idx];

        tatsum += tat[idx];
        wtsum += wt[idx];

        p[idx] = -1;
    }

    cout << fixed << setprecision(2);
    cout << "Average TAT: " << tatsum / n << endl;
    cout << "Average WT: " << wtsum / n << endl;

    return 0;
}
