#include <bits/stdc++.h>
using namespace std;

struct process
{
    int pid;
    int arrivalTime;
    int burstTime;
    int priority;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
};

void findPriority(process processes[], int n)
{
    int currentTime = 0;
    int completed = 0;

    while (completed < n)
    {
        int highestPriority = -1;
        int minPriority = INT_MAX;

        for (int i = 0; i < n; i++)
        {
            if (processes[i].completionTime == 0 &&
                processes[i].arrivalTime <= currentTime &&
                processes[i].priority < minPriority)
            {
                minPriority = processes[i].priority;
                highestPriority = i;
            }
        }

        if (highestPriority == -1)
        {
            currentTime++;
        }
        else
        {
            currentTime += processes[highestPriority].burstTime;

            processes[highestPriority].completionTime = currentTime;

            processes[highestPriority].turnaroundTime =
                processes[highestPriority].completionTime -
                processes[highestPriority].arrivalTime;

            processes[highestPriority].waitingTime =
                processes[highestPriority].turnaroundTime -
                processes[highestPriority].burstTime;

            completed++;
        }
    }
}

void printResult(process processes[], int n)
{
    cout << "\nPID\tAT\tBT\tP\tCT\tTAT\tWT\n";

    for (int i = 0; i < n; i++)
    {
        cout << processes[i].pid << "\t"
             << processes[i].arrivalTime << "\t"
             << processes[i].burstTime << "\t"
             << processes[i].priority << "\t"
             << processes[i].completionTime << "\t"
             << processes[i].turnaroundTime << "\t"
             << processes[i].waitingTime << endl;
    }
}

int main()
{
    int n;

    cout << "Enter number of processes: ";
    cin >> n;

    process processes[n];

    for (int i = 0; i < n; i++)
    {
        processes[i].pid = i + 1;

        cout << "Enter Arrival Time, Burst Time, Priority for P"
             << i + 1 << ": ";

        cin >> processes[i].arrivalTime
            >> processes[i].burstTime
            >> processes[i].priority;

        processes[i].completionTime = 0;
        processes[i].turnaroundTime = 0;
        processes[i].waitingTime = 0;
    }

    findPriority(processes, n);

    printResult(processes, n);

    return 0;
}