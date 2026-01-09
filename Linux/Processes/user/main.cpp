#include <bits/stdc++.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

bool processExists(pid_t pid) {
    if (pid <= 0) return false;
    
    string statusPath = "/proc/" + to_string(pid) + "/status";
    ifstream statusFile(statusPath);
    
    if (!statusFile.is_open()) {
        return false;
    }
    
    string line;
    while (getline(statusFile, line)) {
        if (line.compare(0, 6, "State:") == 0) {
            if (line.find('Z') != string::npos) {
                return false;
            }
            if (kill(pid, 0) == 0 || errno == EPERM) {
                return true;
            }
            return false;
        }
    }
    
    return false;
}

pid_t spawnSleepProcess(int seconds = 300) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        setsid();

        int nullFd = open("/dev/null", O_RDWR);
        if (nullFd != -1) {
            dup2(nullFd, STDIN_FILENO);
            dup2(nullFd, STDOUT_FILENO);
            dup2(nullFd, STDERR_FILENO);
            close(nullFd);
        }

        execl("/bin/sleep", "sleep", to_string(seconds).c_str(), (char*)nullptr);

        perror("execl failed");
        _exit(127);
    }
    
    return pid;
}

int runKillerWithArgs(const vector<string>& args, int &exitStatus) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        vector<char*> argv;
        argv.push_back(const_cast<char*>("./killer"));
        
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execv("./killer", argv.data());

        perror("execv failed");
        _exit(127);
    }

    int status;
    waitpid(pid, &status, 0);
    
    if (WIFEXITED(status)) {
        exitStatus = WEXITSTATUS(status);
    } else {
        exitStatus = -1;
    }
    
    return pid;
}

void shortDelay() { 
    usleep(200000);
}

int main() {
    int exitStatus;
    cout << "=== Process Killer Demo ===\n\n";
    
    cout << "Demo 1: Kill by PID (--id)\n";
    cout << string(40, '-') << "\n";
    
    pid_t sleepPid1 = spawnSleepProcess(300);
    if (sleepPid1 <= 0) {
        cerr << "Failed to spawn sleep process for demo 1\n";
        return 1;
    }
    
    cout << "Spawned sleep process with PID: " << sleepPid1 << "\n";
    shortDelay();
    
    cout << "Process exists before killer: " 
         << (processExists(sleepPid1) ? "YES" : "NO") << "\n";
    
    cout << "\nRunning: ./killer --id " << sleepPid1 << "\n";
    runKillerWithArgs({"--id", to_string(sleepPid1)}, exitStatus);
    
    shortDelay();
    waitpid(sleepPid1, nullptr, 0);
    
    cout << "Process exists after killer: " 
         << (processExists(sleepPid1) ? "YES" : "NO") << "\n";
    cout << "Killer exit status: " << exitStatus << "\n\n";
    
    cout << "Demo 2: Kill by name (--name)\n";
    cout << string(40, '-') << "\n";
    
    pid_t sleepPid2 = spawnSleepProcess(300);
    pid_t sleepPid3 = spawnSleepProcess(300);
    
    if (sleepPid2 <= 0 || sleepPid3 <= 0) {
        cerr << "Failed to spawn sleep processes for demo 2\n";
        return 1;
    }
    
    cout << "Spawned 2 sleep processes with PIDs: " 
         << sleepPid2 << ", " << sleepPid3 << "\n";
    shortDelay();
    
    cout << "Processes exist before killer: "
         << (processExists(sleepPid2) ? "YES" : "NO") << ", "
         << (processExists(sleepPid3) ? "YES" : "NO") << "\n";
    
    cout << "\nRunning: ./killer --name sleep\n";
    runKillerWithArgs({"--name", "sleep"}, exitStatus);
    
    shortDelay();
    waitpid(sleepPid2, nullptr, 0);
    waitpid(sleepPid3, nullptr, 0);
    
    cout << "Processes exist after killer: "
         << (processExists(sleepPid2) ? "YES" : "NO") << ", "
         << (processExists(sleepPid3) ? "YES" : "NO") << "\n";
    cout << "Killer exit status: " << exitStatus << "\n\n";
    
    cout << "Demo 3: Kill using PROCTOKILL env var (single name)\n";
    cout << string(40, '-') << "\n";
    
    setenv("PROCTOKILL", "sleep", 1);
    cout << "Set PROCTOKILL='sleep'\n";
    
    pid_t sleepPid4 = spawnSleepProcess(300);
    pid_t sleepPid5 = spawnSleepProcess(300);
    
    if (sleepPid4 <= 0 || sleepPid5 <= 0) {
        cerr << "Failed to spawn sleep processes for demo 3\n";
        return 1;
    }
    
    cout << "Spawned 2 sleep processes with PIDs: "
         << sleepPid4 << ", " << sleepPid5 << "\n";
    shortDelay();
    
    cout << "Processes exist before killer: "
         << (processExists(sleepPid4) ? "YES" : "NO") << ", "
         << (processExists(sleepPid5) ? "YES" : "NO") << "\n";
    
    cout << "\nRunning: ./killer (no arguments, reads PROCTOKILL)\n";
    runKillerWithArgs({}, exitStatus);
    
    shortDelay();
    waitpid(sleepPid4, nullptr, 0);
    waitpid(sleepPid5, nullptr, 0);
    
    cout << "Processes exist after killer: "
         << (processExists(sleepPid4) ? "YES" : "NO") << ", "
         << (processExists(sleepPid5) ? "YES" : "NO") << "\n";
    cout << "Killer exit status: " << exitStatus << "\n\n";

    cout << "Demo 4: Kill using PROCTOKILL env var (multiple names)\n";
    cout << string(40, '-') << "\n";
    
    setenv("PROCTOKILL", "sleep,nonexistent", 1);
    cout << "Set PROCTOKILL='sleep,nonexistent'\n";
    
    pid_t sleepPid6 = spawnSleepProcess(300);
    
    if (sleepPid6 <= 0) {
        cerr << "Failed to spawn sleep process for demo 4\n";
        return 1;
    }
    
    cout << "Spawned sleep process with PID: " << sleepPid6 << "\n";
    shortDelay();
    
    cout << "Process exists before killer: "
         << (processExists(sleepPid6) ? "YES" : "NO") << "\n";
    
    cout << "\nRunning: ./killer (no arguments, reads PROCTOKILL)\n";
    runKillerWithArgs({}, exitStatus);
    
    shortDelay();
    waitpid(sleepPid6, nullptr, 0);
    
    cout << "Process exists after killer: "
         << (processExists(sleepPid6) ? "YES" : "NO") << "\n";
    cout << "Killer exit status: " << exitStatus << "\n\n";
    
    cout << "Cleaning up environment...\n";
    unsetenv("PROCTOKILL");
    
    if (getenv("PROCTOKILL") == nullptr) {
        cout << "PROCTOKILL environment variable removed successfully\n";
    } else {
        cout << "Warning: PROCTOKILL still exists: " << getenv("PROCTOKILL") << "\n";
    }
    
    cout << "\n=== Demo finished ===\n";
    return 0;
}