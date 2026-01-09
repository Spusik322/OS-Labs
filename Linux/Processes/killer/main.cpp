#include <bits/stdc++.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>

using namespace std;

static pid_t getSelfPid() { return getpid(); }

string getProcessName(pid_t pid) {
    string commPath = "/proc/" + to_string(pid) + "/comm";
    ifstream commFile(commPath);
    if (commFile.is_open()) {
        string name;
        getline(commFile, name);
        if (!name.empty()) return name;
    }
    
    string cmdPath = "/proc/" + to_string(pid) + "/cmdline";
    ifstream cmdFile(cmdPath, ios::binary);
    if (!cmdFile.is_open()) return "";
    
    string cmdline;
    char ch;
    while (cmdFile.get(ch) && ch != '\0') {
        cmdline += ch;
    }
    
    if (!cmdline.empty()) {
        size_t slash = cmdline.find_last_of('/');
        if (slash != string::npos) {
            return cmdline.substr(slash + 1);
        }
        return cmdline;
    }
    
    return "";
}

vector<pid_t> findProcessesByName(const string &name) {
    vector<pid_t> result;
    DIR *procDir = opendir("/proc");
    if (!procDir) return result;
    
    struct dirent *entry;
    pid_t self = getSelfPid();
    
    while ((entry = readdir(procDir)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;
        
        bool isPid = true;
        for (int i = 0; entry->d_name[i]; i++) {
            if (!isdigit(entry->d_name[i])) {
                isPid = false;
                break;
            }
        }
        if (!isPid) continue;
        
        pid_t pid = static_cast<pid_t>(stoi(entry->d_name));
        if (pid <= 1 || pid == self) continue;
        
        string procName = getProcessName(pid);
        if (procName.empty()) continue;
        
        if (procName == name) {
            result.push_back(pid);
            continue;
        }

        string cmdPath = "/proc/" + to_string(pid) + "/cmdline";
        ifstream cmdFile(cmdPath, ios::binary);
        if (cmdFile.is_open()) {
            string cmdline;
            char ch;
            while (cmdFile.get(ch) && ch != '\0') {
                cmdline += ch;
            }
            
            if (!cmdline.empty()) {
                string baseName = cmdline;
                size_t slash = baseName.find_last_of('/');
                if (slash != string::npos) {
                    baseName = baseName.substr(slash + 1);
                }
                
                if (baseName == name) {
                    result.push_back(pid);
                }
            }
        }
    }
    
    closedir(procDir);

    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    
    return result;
}

bool isProcessRunning(pid_t pid) {
    if (pid <= 0) return false;
    return (kill(pid, 0) == 0 || errno == EPERM);
}

bool killProcessGracefully(pid_t pid) {
    if (!isProcessRunning(pid)) {
        cout << "Process " << pid << " is not running\n";
        return true;
    }
    
    string procName = getProcessName(pid);
    if (!procName.empty()) {
        cout << "Terminating process " << pid << " (" << procName << ")\n";
    } else {
        cout << "Terminating process " << pid << "\n";
    }

    if (kill(pid, SIGTERM) != 0) {
        if (errno == ESRCH) {
            cout << "Process " << pid << " already terminated\n";
            return true;
        }
        cerr << "Failed to send SIGTERM: " << strerror(errno) << "\n";
        return false;
    }

    for (int i = 0; i < 20; ++i) {
        if (!isProcessRunning(pid)) {
            cout << "Process " << pid << " terminated gracefully\n";
            return true;
        }
        usleep(100000);
    }

    cout << "Sending SIGKILL to process " << pid << "\n";
    
    if (kill(pid, SIGKILL) != 0) {
        if (errno == ESRCH) {
            cout << "Process " << pid << " terminated after SIGTERM\n";
            return true;
        }
        cerr << "Failed to send SIGKILL: " << strerror(errno) << "\n";
        return false;
    }
    
    usleep(100000);
    
    if (!isProcessRunning(pid)) {
        cout << "Process " << pid << " killed with SIGKILL\n";
        return true;
    }
    
    cerr << "Failed to kill process " << pid << "\n";
    return false;
}

vector<string> splitCommaSeparated(const string &s) {
    vector<string> result;
    string current;
    bool inQuotes = false;
    char quoteChar = 0;
    
    for (char c : s) {
        if (inQuotes) {
            if (c == quoteChar) {
                inQuotes = false;
            } else {
                current += c;
            }
        } else {
            if (c == '\'' || c == '"') {
                inQuotes = true;
                quoteChar = c;
            } else if (c == ',') {
                if (!current.empty()) {
                    size_t start = current.find_first_not_of(" \t\r\n");
                    size_t end = current.find_last_not_of(" \t\r\n");
                    if (start != string::npos) {
                        result.push_back(current.substr(start, end - start + 1));
                    }
                }
                current.clear();
            } else {
                current += c;
            }
        }
    }

    if (!current.empty()) {
        size_t start = current.find_first_not_of(" \t\r\n");
        size_t end = current.find_last_not_of(" \t\r\n");
        if (start != string::npos) {
            result.push_back(current.substr(start, end - start + 1));
        }
    }
    
    return result;
}

void killByName(const string &name) {
    cout << "Searching for processes named '" << name << "'\n";
    
    auto pids = findProcessesByName(name);
    if (pids.empty()) {
        cout << "No processes found with name '" << name << "'\n";
        return;
    }
    
    cout << "Found " << pids.size() << " process(es):";
    for (pid_t pid : pids) cout << " " << pid;
    cout << "\n";
    
    int success = 0;
    for (pid_t pid : pids) {
        if (killProcessGracefully(pid)) {
            success++;
        }
    }
    
    cout << "Successfully terminated " << success << "/" << pids.size() << " processes\n";
}

int main(int argc, char **argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string optId, optName;
    
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        
        if (arg == "--id" || arg == "-p") {
            if (i + 1 >= argc) {
                cerr << "Error: Missing PID argument\n";
                return 1;
            }
            optId = argv[++i];
        } 
        else if (arg == "--name" || arg == "-n") {
            if (i + 1 >= argc) {
                cerr << "Error: Missing name argument\n";
                return 1;
            }
            optName = argv[++i];
        }
        else if (arg == "--help" || arg == "-h") {
            cout << "Usage: killer [OPTIONS]\n"
                 << "Options:\n"
                 << "  --id, -p PID    Kill process by PID\n"
                 << "  --name, -n NAME Kill processes by name\n"
                 << "  --help, -h      Show this help\n\n"
                 << "If no arguments, reads PROCTOKILL environment variable\n"
                 << "(comma-separated process names).\n";
            return 0;
        }
        else {
            cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    if (!optId.empty()) {
        try {
            pid_t pid = static_cast<pid_t>(stol(optId));
            
            if (pid == getSelfPid()) {
                cerr << "Refusing to kill self (pid " << pid << ")\n";
                return 1;
            }
            
            cout << "Attempting to kill pid " << pid << "\n";
            if (killProcessGracefully(pid)) {
                cout << "Successfully killed pid " << pid << "\n";
                return 0;
            } else {
                cerr << "Failed to kill pid " << pid << "\n";
                return 2;
            }
        } catch (const exception &e) {
            cerr << "Invalid PID: " << optId << "\n";
            return 1;
        }
    }

    if (!optName.empty()) {
        killByName(optName);
        return 0;
    }
    
    const char *env = getenv("PROCTOKILL");
    if (!env || env[0] == '\0') {
        cerr << "No arguments provided and PROCTOKILL is not set\n"
             << "Use --help for usage information\n";
        return 3;
    }
    
    cout << "Using PROCTOKILL: " << env << "\n";
    auto names = splitCommaSeparated(env);
    
    if (names.empty()) {
        cerr << "PROCTOKILL is empty or malformed\n";
        return 4;
    }
    
    int totalFound = 0, totalKilled = 0;
    
    for (const auto &name : names) {
        if (name.empty()) continue;
        
        cout << "\nLooking for processes named '" << name << "'\n";
        auto pids = findProcessesByName(name);
        
        if (pids.empty()) {
            cout << "  No processes found for '" << name << "'\n";
            continue;
        }
        
        totalFound += pids.size();
        cout << "  Found " << pids.size() << " process(es):";
        for (pid_t pid : pids) cout << " " << pid;
        cout << "\n";
        
        for (pid_t pid : pids) {
            if (killProcessGracefully(pid)) {
                totalKilled++;
            }
        }
    }
    
    cout << "\nSummary: Found " << totalFound << " processes, successfully terminated " << totalKilled << "\n";
    return (totalKilled > 0) ? 0 : 5;
}