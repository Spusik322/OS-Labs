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

    string procPath = "/proc/" + to_string(pid);
    struct stat st;
    if (stat(procPath.c_str(), &st) != 0) {
        return false;
    }

    string statPath = procPath + "/stat";
    ifstream statFile(statPath);
    if (!statFile.is_open()) {
        return false;
    }
    
    string line;
    getline(statFile, line);

    size_t lastParen = line.find_last_of(')');
    if (lastParen == string::npos) return false;
    
    stringstream ss(line.substr(lastParen + 1));
    string stateStr;
    ss >> stateStr;
    
    if (stateStr.empty()) return false;
    
    char state = stateStr[0];
    return !(state == 'Z' || state == 'X' || state == 'x');
}

pid_t spawnSleepProcess(int seconds = 5) {
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

        char* argv[] = {(char*)"sleep", (char*)to_string(seconds).c_str(), nullptr};
        execvp("sleep", argv);

        perror("execvp failed");
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
    usleep(500000);
}

void cleanupZombies(pid_t pid) {
    if (pid > 0) {
        int status;
        waitpid(pid, &status, WNOHANG);
    }
}

int main() {
    int exitStatus;
    cout << "=== Process Killer Demo ===\n\n";

    cout << "Demo 1: Kill by PID (--id)\n";
    cout << string(40, '-') << "\n";
    
    pid_t sleepPid1 = spawnSleepProcess(5);
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
    cleanupZombies(sleepPid1);
    
    cout << "Process exists after killer: " 
         << (processExists(sleepPid1) ? "YES" : "NO") << "\n";
    cout << "Killer exit status: " << exitStatus << "\n\n";

    cout << "Demo 2: Kill by name (--name)\n";
    cout << string(40, '-') << "\n";
    
    pid_t sleepPid2 = spawnSleepProcess(5);
    pid_t sleepPid3 = spawnSleepProcess(5);
    
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
    cleanupZombies(sleepPid2);
    cleanupZombies(sleepPid3);
    
    cout << "Processes exist after killer: "
         << (processExists(sleepPid2) ? "YES" : "NO") << ", "
         << (processExists(sleepPid3) ? "YES" : "NO") << "\n";
    cout << "Killer exit status: " << exitStatus << "\n\n";
    
    cout << "Demo 3: Kill using PROCTOKILL env var (single name)\n";
    cout << string(40, '-') << "\n";
    
    setenv("PROCTOKILL", "sleep", 1);
    cout << "Set PROCTOKILL='sleep'\n";
    
    pid_t sleepPid4 = spawnSleepProcess(5);
    pid_t sleepPid5 = spawnSleepProcess(5);
    
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
    cleanupZombies(sleepPid4);
    cleanupZombies(sleepPid5);
    
    cout << "Processes exist after killer: "
         << (processExists(sleepPid4) ? "YES" : "NO") << ", "
         << (processExists(sleepPid5) ? "YES" : "NO") << "\n";
    cout << "Killer exit status: " << exitStatus << "\n\n";

    cout << "Demo 4: Kill using PROCTOKILL env var (multiple names)\n";
    cout << string(40, '-') << "\n";
    
    setenv("PROCTOKILL", "sleep,nonexistent", 1);
    cout << "Set PROCTOKILL='sleep,nonexistent'\n";
    
    pid_t sleepPid6 = spawnSleepProcess(5);
    
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
    cleanupZombies(sleepPid6);
    
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