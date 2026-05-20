#include <iostream>
#include <fstream>
#include <unistd.h>
#include <atomic>
#include <csignal>
#include <chrono>
#include <thread>
#include <sstream>

using namespace std;

atomic<bool> keep_running(true);

void SignalHandler (int sig_num) {
    if(sig_num == SIGINT) {
        keep_running = false;
    }
}
int GetPID () {
    int pid_num;

    ifstream file ("video_clipper.pid");
    file >> pid_num;
    return pid_num;
}

float GetSystemMem() {  

    ifstream file ("/proc/meminfo");
    string line;
    long total = 0;
    long available = 0;

    while (getline(file, line)) {
        
        stringstream ss(line);
        string key;
        long value;
        
        ss >> key >> value;
        
        if (key == "MemTotal:")
            total = value;

        if (key == "MemAvailable:")
            available = value;

    }
    return 100.0f * (total - available) / total;
 
}

long GetProcessMem(int pid) {

    ifstream file("/proc/" + to_string(pid)+ "/status");
    string line;

    while (getline(file, line)) {

        if (line.find("VmRSS:") != string::npos)
        {
            stringstream ss(line);
            string tmp;
            long mem;

            ss >> tmp >> mem;

            return mem;
        }
    }

    return 0;
}
int main() {

    signal(SIGINT,SignalHandler);
    int process_pid = 0;
    float mem_usage = 0.0;
    long proc_mem = 0;
    process_pid = GetPID ();
    
    while(keep_running) {
        // Get CPU Usage /proc/stat and Memmory Info /proc/meminfo and print every 1 s
        //process : /proc/[PID]/stat /proc/[PID]/status
        cout << "======== CPU WATCH ========" << endl;;
        mem_usage = GetSystemMem();
        cout << "System Memory Usage: " << mem_usage << endl;
        proc_mem = GetProcessMem(process_pid);
        cout << "Video Clipper Memory(RAM): " << proc_mem / 1024.0 << " MB " << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    return 0;
}
