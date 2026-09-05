#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Process {
    string id;
    int arrivalTime;
    int burstTime;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
};

int main() {

    int n;

    cout << "Enter number of processes: ";
    cin >> n;

    vector<Process> p(n);

    // Input
    for (int i = 0; i < n; i++) {
        cout << "\nProcess " << i + 1 << endl;

        cout << "Process ID: ";
        cin >> p[i].id;

        cout << "Arrival Time: ";
        cin >> p[i].arrivalTime;

        cout << "Burst Time: ";
        cin >> p[i].burstTime;
    }

    // Sort according to Arrival Time
    sort(p.begin(), p.end(), [](Process a, Process b) {
        return a.arrivalTime < b.arrivalTime;
    });

    int currentTime = 0;
    double totalWaitingTime = 0;
    double totalTurnaroundTime = 0;

    // FCFS calculation
    for (int i = 0; i < n; i++) {

        // CPU remains idle if process has not arrived
        if (currentTime < p[i].arrivalTime) {
            currentTime = p[i].arrivalTime;
        }

        // Process executes completely
        currentTime += p[i].burstTime;

        // Completion Time
        p[i].completionTime = currentTime;

        // Turnaround Time
        p[i].turnaroundTime =
            p[i].completionTime - p[i].arrivalTime;

        // Waiting Time
        p[i].waitingTime =
            p[i].turnaroundTime - p[i].burstTime;

        totalWaitingTime += p[i].waitingTime;
        totalTurnaroundTime += p[i].turnaroundTime;
    }

    // Output
    cout << "\n\nFCFS Scheduling\n";

    cout << "-------------------------------------------------------------\n";

    cout << "PID\tAT\tBT\tCT\tTAT\tWT\n";

    cout << "-------------------------------------------------------------\n";

    for (int i = 0; i < n; i++) {
        cout << p[i].id << "\t"
             << p[i].arrivalTime << "\t"
             << p[i].burstTime << "\t"
             << p[i].completionTime << "\t"
             << p[i].turnaroundTime << "\t"
             << p[i].waitingTime << endl;
    }

    cout << "-------------------------------------------------------------\n";

    cout << "Average Waiting Time: "
         << totalWaitingTime / n << endl;

    cout << "Average Turnaround Time: "
         << totalTurnaroundTime / n << endl;

    return 0;
}