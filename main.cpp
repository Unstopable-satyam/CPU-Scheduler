#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <set>
#include <unordered_map>
#include <numeric>
#include <climits>
using namespace std;

// Class to represent a task (previously called Process)
class Task {
public:
    int id;                 // Task ID
    int burst;              // Burst time of the task
    int arrival;            // Arrival time of the task
    int remaining;          // Remaining time of the task
    int wait;               // Waiting time of the task
    int turnaround;         // Turnaround time of the task

    Task(int id, int burst, int arrival)
        : id(id), burst(burst), arrival(arrival), remaining(burst), wait(0), turnaround(0) {}
};

// Class to represent the execution of a task
class Execution {
public:
    int id;                 // Task ID
    int start;              // Start time of the task execution
    int end;                // End time of the task execution

    Execution(int id, int start, int end)
        : id(id), start(start), end(end) {}
};


bool compareByArrival(const Task& a, const Task& b) {
    return a.arrival < b.arrival;
}

// Function to predict the best scheduling algorithm based on task characteristics
string predictBestAlgorithm(vector<Task>& tasks) {
    // Calculate total burst time and average burst time
    double totalBurstTime = 0.0;
    for (const auto& task : tasks) {
        totalBurstTime += task.burst;
    }
    double averageBurstTime = totalBurstTime / tasks.size();

    // Check if all burst times are the same
    bool areAllBurstTimesSame = true;
    for (const auto& task : tasks) {
        if (task.burst != tasks[0].burst) {
            areAllBurstTimesSame = false;
            break;
        }
    }

    if (areAllBurstTimesSame) {
        return "FCFS";
    }

    // Sort tasks by arrival time using the comparator function
    std::sort(tasks.begin(), tasks.end(), compareByArrival);

    // Check for overlapping arrival times
    bool hasOverlappingTimes = false;
    for (size_t i = 0; i < tasks.size() - 1; ++i) {
        if (tasks[i].arrival + tasks[i].burst > tasks[i + 1].arrival) {
            hasOverlappingTimes = true;
            break;
        }
    }

    if (hasOverlappingTimes) {
        return "SRTF";
    } else {
        return "SJF";
    }
}

// Function to calculate waiting time and turnaround time for each task
void printAverageTimes(std::vector<Task>& tasks, std::ofstream& outputFile) {
    int totalWait = 0, totalTurnaround = 0;

    for (int i = 0; i < tasks.size(); ++i) {
        tasks[i].turnaround = tasks[i].wait + tasks[i].burst;
        totalWait += tasks[i].wait;
        totalTurnaround += tasks[i].turnaround;
    }

    outputFile << "Average Waiting Time: " << (float)totalWait / tasks.size() << std::endl;
    outputFile << "Average Turnaround Time: " << (float)totalTurnaround / tasks.size() << std::endl;
}

// Function to print the Gantt chart of task executions
void printGanttChart(vector<Execution>& executions, ofstream& outputFile) {
    if (executions.empty()) return;

    outputFile << "Gantt Chart:" << endl;

    // Compress consecutive executions of the same task and handle gaps
    vector<Execution> compressedExecutions;
    Execution current = executions[0];

    for (int i = 1; i < executions.size(); ++i) {
        if (executions[i].id == current.id && executions[i].start == current.end) {
            current.end = executions[i].end;
        } else {
            compressedExecutions.push_back(current);
            if (executions[i].start > current.end) {
                compressedExecutions.push_back(Execution(-1, current.end, executions[i].start)); // Gap
            }
            current = executions[i];
        }
    }
    compressedExecutions.push_back(current);

    // Print the Gantt chart
    for (const auto& e : compressedExecutions) {
        if (e.id == -1) {
            outputFile << "|       ";
        } else {
            outputFile << "|  T" << e.id << "   ";
        }
    }
    outputFile << "|" << endl;

    for (const auto& e : compressedExecutions) {
        outputFile << e.start << "\t";
    }
    outputFile << compressedExecutions.back().end << endl << endl;
}

// Function to implement First-Come, First-Served (FCFS) scheduling algorithm
void fcfs(std::vector<Task> tasks, std::ofstream& outputFile) {
    std::sort(tasks.begin(), tasks.end(), compareByArrival);

    std::vector<Execution> executions;
    int currentTime = 0;

    for (int i = 0; i < tasks.size(); ++i) {
        if (currentTime < tasks[i].arrival) {
            currentTime = tasks[i].arrival;
        }
        tasks[i].wait = std::max(0, currentTime - tasks[i].arrival);
        executions.push_back(Execution{tasks[i].id, currentTime, currentTime + tasks[i].burst});
        currentTime += tasks[i].burst;
    }

    // Print average times and Gantt chart
    outputFile << "FCFS Scheduling:" << std::endl;
    printAverageTimes(tasks, outputFile);
    printGanttChart(executions, outputFile);
}

// Corrected Function to implement Shortest Job First (SJF) scheduling algorithm
void sjf(vector<Task> tasks, ofstream& outputFile) {
    auto compareArrival = [](const Task& a, const Task& b) {
        return a.arrival < b.arrival;
    };
    sort(tasks.begin(), tasks.end(), compareArrival);

    vector<Execution> executions;
    int currentTime = 0;
    int completedTasks = 0;
    int n = tasks.size();
    vector<bool> isTaskCompleted(n, false);

    while (completedTasks < n) {
        int shortestJobIndex = -1;
        int shortestBurstTime = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (tasks[i].arrival <= currentTime && !isTaskCompleted[i] && tasks[i].burst < shortestBurstTime) {
                shortestBurstTime = tasks[i].burst;
                shortestJobIndex = i;
            }
        }

        if (shortestJobIndex == -1) {
            currentTime++;
            continue;
        }

        Task& t = tasks[shortestJobIndex];
        t.wait = max(0, currentTime - t.arrival);
        executions.push_back(Execution(t.id, currentTime, currentTime + t.burst));
        currentTime += t.burst;
        t.turnaround = t.wait + t.burst;
        isTaskCompleted[shortestJobIndex] = true;
        completedTasks++;
    }

    outputFile << "SJF Scheduling:" << endl;
    printAverageTimes(tasks, outputFile);
    printGanttChart(executions, outputFile);
}

// Function to implement Shortest Remaining Time First (SRTF) scheduling algorithm
void srtf(vector<Task> tasks, ofstream& outputFile) {
    int n = tasks.size();
    vector<Execution> executions;
    int currentTime = 0;
    int completed = 0;
    int shortestIndex = 0;
    int shortestRemainingTime = INT_MAX;
    bool check = false;

    while (completed != n) {
        for (int i = 0; i < n; ++i) {
            if (tasks[i].arrival <= currentTime && tasks[i].remaining > 0) {
                if (tasks[i].remaining < shortestRemainingTime) {
                    shortestRemainingTime = tasks[i].remaining;
                    shortestIndex = i;
                    check = true;
                }
            }
        }

        if (!check) {
            currentTime++;
            continue;
        }

        // Execute the task for 1 unit of time
        executions.push_back(Execution(tasks[shortestIndex].id, currentTime, currentTime + 1));
        tasks[shortestIndex].remaining--;

        shortestRemainingTime = tasks[shortestIndex].remaining;
        if (shortestRemainingTime == 0) {
            shortestRemainingTime = INT_MAX;
        }

        // Check if the task has completed
        if (tasks[shortestIndex].remaining == 0) {
            completed++;
            check = false;

            tasks[shortestIndex].wait = max(0, currentTime + 1 - tasks[shortestIndex].arrival - tasks[shortestIndex].burst);
            tasks[shortestIndex].turnaround = tasks[shortestIndex].wait + tasks[shortestIndex].burst;
        }

        currentTime++;
    }

    outputFile << "SRTF Scheduling:" << endl;
    printAverageTimes(tasks, outputFile);
    printGanttChart(executions, outputFile);
}

// Function to calculate the 80th percentile of burst times
int calculateQuantum(vector<int>& bursts) {
    sort(bursts.begin(), bursts.end());
    int index = static_cast<int>(bursts.size() * 0.8);
    return bursts[index];
}



// Function to implement Round Robin scheduling algorithm with dynamic quantum estimation
void roundRobin(vector<Task> tasks, int quantum, ofstream& outputFile) {
    vector<Execution> executions;
    int currentTime = 0;
    queue<int> readyQueue;
    int completed = 0;

    for (auto& t : tasks) {
        t.remaining = t.burst;
    }

    int n = tasks.size();
    int arrival[n];
    for (int i = 0; i < n; i++) {
        arrival[i] = tasks[i].arrival;
    }

    while (completed != tasks.size()) {
        for (int i = 0; i < n; i++) {
            if (arrival[i] <= currentTime && arrival[i] != -1) {
                readyQueue.push(i);
                arrival[i] = -1;
            }
        }

        if (readyQueue.empty()) {
            currentTime++;
            continue;
        }

        int current = readyQueue.front();
        readyQueue.pop();

        int timeSlice = min(quantum, tasks[current].remaining);
        executions.push_back(Execution(tasks[current].id, currentTime, currentTime + timeSlice));
        tasks[current].remaining -= timeSlice;
        currentTime += timeSlice;

        for (int i = 0; i < n; i++) {
            if (arrival[i] <= currentTime && arrival[i] != -1) {
                readyQueue.push(i);
                arrival[i] = -1;
            }
        }

        if (tasks[current].remaining > 0) {
            readyQueue.push(current);
        } else {
            completed++;
            tasks[current].wait = currentTime - tasks[current].arrival - tasks[current].burst;
        }
    }

    outputFile << "Round Robin Scheduling (Dynamic Quantum):" << endl;
    outputFile << "Estimated Quantum Time: " << quantum << endl;
    printAverageTimes(tasks, outputFile);
    printGanttChart(executions, outputFile);
}


// Main function to read input, predict best algorithm, and run all scheduling algorithms
int main() {
    ifstream inputFile("input.txt");
    ofstream outputFile("output.txt");

    if (!inputFile.is_open()) {
        cerr << "Error reading input file" << endl;
        return 0;
    }
    if (!outputFile.is_open()) {
        cerr << "Error reading output file" << endl;
        return 0;
    }

    vector<Task> tasks;
    int id, burst, arrival;

    while (inputFile >> id >> burst >> arrival) {
        tasks.push_back(Task(id, burst, arrival));
    }

    if (tasks.empty()) {
        outputFile << "No tasks found in input file" << endl;
        return 0;
    } else {
        for (const auto& t : tasks) {
            cerr << "Loaded Task - ID: " << t.id << ", Burst Time: " << t.burst << ", Arrival Time: " << t.arrival << endl;
        }
    }

    string predictedAlgorithm = predictBestAlgorithm(tasks);
    outputFile << "Algorithm having minimum average time is " << predictedAlgorithm << endl;
    outputFile << endl;

    vector<int> burstTimes;
    for (const auto& t : tasks) {
        burstTimes.push_back(t.burst);
    }
    int quantum = calculateQuantum(burstTimes);
    roundRobin(tasks, quantum, outputFile);
    fcfs(tasks, outputFile);
    sjf(tasks, outputFile);
    srtf(tasks, outputFile);

   
    inputFile.close();
    outputFile.close();
    return 0;
}
