#include <bits/stdc++.h>
using namespace std;

struct process
{
    int processid;
    int arivalTime;
    int burstTime;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
};
void findNonPreemptiveSJF(process pro[], int n)
{
    int currentTime = 0;
    int complete = 0;

    while (complete < n)
    {
        int shortestBurstTime = -1;
        int minBurstTime = INT_MAX;

        for (int i = 0; i < n; i++)
        {
            if (pro[i].arivalTime <= currentTime &&
                pro[i].completionTime == 0 &&
                pro[i].burstTime < minBurstTime)
            {
                minBurstTime = pro[i].burstTime;
                shortestBurstTime = i;
            }
        }

        if (shortestBurstTime == -1)
        {
            currentTime++;
            continue;
        }
        currentTime += pro[shortestBurstTime].burstTime;

        pro[shortestBurstTime].completionTime = currentTime;

        pro[shortestBurstTime].turnaroundTime =
            pro[shortestBurstTime].completionTime -
            pro[shortestBurstTime].arivalTime;

        pro[shortestBurstTime].waitingTime =
            pro[shortestBurstTime].turnaroundTime -
            pro[shortestBurstTime].burstTime;

        complete++;
    }
}
void findSJF(process pro[],int n){
    vector<int> remainingTime(n);
    for (int i = 0; i < n; i++) {
        remainingTime[i] = pro[i].burstTime;
    }

    int currentTime = 0;
    int complete = 0;
    
    

    while(complete<n){
        int minBurstTime = 9999;
        int shortestBurstTime = -1;
        for(int i = 0;i<n;i++){
            if(pro[i].arivalTime<=currentTime && remainingTime[i]<minBurstTime && remainingTime[i]>0 ){
                minBurstTime = remainingTime[i];
                shortestBurstTime = i;
            }
        }
        if(shortestBurstTime == -1){
            currentTime++;
            continue;
        }
        remainingTime[shortestBurstTime]--;
        if(remainingTime[shortestBurstTime] == 0){
            complete++;
            pro[shortestBurstTime].completionTime = currentTime + 1;
            pro[shortestBurstTime].turnaroundTime = pro[shortestBurstTime].completionTime - pro[shortestBurstTime].arivalTime;
            pro[shortestBurstTime].waitingTime = pro[shortestBurstTime].turnaroundTime - pro[shortestBurstTime].burstTime;
         }
         currentTime++;
    }
}
void displayProcessDetails(process proc[], int n){
    cout<<"Process\tArrivalTime\t\tBurstTime\tCompletionTime\tTurnaroundTime\tWaitingTime"<<endl;
    for (int i = 0; i < n; i++)
    {
        cout<<proc[i].processid<<"\t"<<proc[i].arivalTime<<"\t"<<proc[i].burstTime<<"\t"<<proc[i].completionTime<<"\t\t"<<proc[i].turnaroundTime<<"\t\t"<<proc[i].waitingTime<<endl;
    }
    cout<<endl;
}


int main()
{
    int n;
    cin >> n;
    process pro[n];
    for (int i = 0; i < n; i++)
    {
        cout << "Enter Process: " << i + 1 << endl;
        cin >> pro[i].processid >> pro[i].arivalTime>> pro[i].burstTime;
        pro[i].completionTime = 0;
    }
    findSJF(pro,n);
    // displayProcessDetails(pro,n);
    // findNonPreemptiveSJF(pro,n);
    displayProcessDetails(pro,n);
}
/*
5
1 0 5
2 2 3
3 1 8
4 4 2
5 3 4
*/