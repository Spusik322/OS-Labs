#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cerrno>

static std::vector<std::string> split_comma(const std::string &s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
        size_t a = item.find_first_not_of(" \t\r\n\"");
        size_t b = item.find_last_not_of(" \t\r\n\"");
        if (a == std::string::npos) continue;
        out.push_back(item.substr(a, b - a + 1));
    }
    return out;
}

static std::vector<pid_t> pgrep_f(const std::string &pattern) {
    std::vector<pid_t> pids;
    std::string cmd = "pgrep -x '" + pattern + "' 2>/dev/null || pgrep -f '" + pattern + "' 2>/dev/null";
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) return pids;
    char buf[128];
    while (fgets(buf, sizeof(buf), fp)) {
        pid_t pid = static_cast<pid_t>(std::atoi(buf));
        if (pid > 0) pids.push_back(pid);
    }
    pclose(fp);
    pid_t me = getpid();
    pids.erase(std::remove(pids.begin(), pids.end(), me), pids.end());
    std::sort(pids.begin(), pids.end());
    pids.erase(std::unique(pids.begin(), pids.end()), pids.end());
    return pids;
}

static bool is_running(pid_t pid) {
    if (pid <= 0) return false;
    return (kill(pid, 0) == 0 || errno == EPERM);
}

static bool kill_graceful_then_force(pid_t pid, int wait_ms = 1500) {
    if (pid <= 0) return false;
    if (kill(pid, SIGTERM) != 0) {
        if (errno == ESRCH) {
            std::cout << "[KILL] PID " << pid << " not found\n";
            return true;
        }
        std::perror("[KILL] kill(SIGTERM) failed");
        return false;
    }
    int waited = 0;
    while (waited < wait_ms) {
        if (!is_running(pid)) {
            std::cout << "[KILL] PID " << pid << " terminated with SIGTERM\n";
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        waited += 100;
    }
    if (kill(pid, SIGKILL) != 0) {
        if (errno == ESRCH) { std::cout << "[KILL] PID " << pid << " gone after SIGTERM\n"; return true; }
        std::perror("[KILL] kill(SIGKILL) failed");
        return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (!is_running(pid)) {
        std::cout << "[KILL] PID " << pid << " killed with SIGKILL\n";
        return true;
    }
    std::cout << "[KILL] Failed to kill PID " << pid << "\n";
    return false;
}

static void kill_by_id(pid_t pid) {
    std::cout << "[KILL] Attempting to kill PID " << pid << "\n";
    kill_graceful_then_force(pid);
}

static void kill_by_name(const std::string &name) {
    std::cout << "[KILL] Searching for processes matching: " << name << "\n";
    auto pids = pgrep_f(name);
    if (pids.empty()) {
        std::cout << "[KILL] No processes found for: " << name << "\n";
        return;
    }
    std::cout << "[KILL] Found pids:";
    for (auto pid : pids) std::cout << " " << pid;
    std::cout << "\n";
    for (pid_t pid : pids) {
        kill_graceful_then_force(pid);
    }
}

static void print_usage() {
    std::cout << "Usage: killer [--id <pid>] [--name <procname>]\n"
              << "If no args provided, reads PROC_TO_KILL env var (comma-separated names)\n";
}

int main(int argc, char** argv) {
    bool processed = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--id") {
            if (i+1 >= argc) { print_usage(); return 1; }
            pid_t pid = static_cast<pid_t>(std::stol(argv[++i]));
            kill_by_id(pid);
            processed = true;
        } else if (a == "--name") {
            if (i+1 >= argc) { print_usage(); return 1; }
            kill_by_name(argv[++i]);
            processed = true;
        } else if (a == "-h" || a == "--help") {
            print_usage();
            return 0;
        } else {
            std::cerr << "Unknown arg: " << a << "\n"; print_usage(); return 1;
        }
    }

    if (!processed) {
        const char* env = std::getenv("PROC_TO_KILL");
        if (env && env[0]) {
            std::string senv(env);
            std::cout << "[KILL] PROC_TO_KILL=" << senv << "\n";
            auto list = split_comma(senv);
            for (auto &name : list) {
                if (!name.empty()) kill_by_name(name);
            }
        } else {
            print_usage();
        }
    }
    return 0;
}
