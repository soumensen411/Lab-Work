#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    cin >> n;

    vector<int> at(n), bt(n), ct(n), tat(n), wt(n);
    vector<bool> done(n, false);

    for (int i = 0; i < n; i++) {
        cin >> at[i] >> bt[i];
    }

    int time = 0, completed = 0;
    vector<int> process;

    while (completed < n) {
        int idx = -1;

        for (int i = 0; i < n; i++) {
            if (!done[i] && at[i] <= time) {
                if (idx == -1 || bt[i] < bt[idx]) {
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        time += bt[idx];
        ct[idx] = time;
        tat[idx] = ct[idx] - at[idx];
        wt[idx] = tat[idx] - bt[idx];

        done[idx] = true;
        completed++;
        process.push_back(idx);
    }

    cout << "Execution Order: ";
    for (int i : process) {
        cout << "P" << i + 1 << " ";
    }
    cout << endl;

    double tatsum = 0, wtsum = 0;

    for (int i = 0; i < n; i++) {
        tatsum += tat[i];
        wtsum += wt[i];
    }

    cout << fixed << setprecision(2);
    cout << "AVG TAT TIME = " << tatsum / n << endl;
    cout << "AVG WT TIME = " << wtsum / n << endl;

    return 0;
}