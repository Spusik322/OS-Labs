#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <algorithm>

static std::string run_cmd_capture(const std::string &cmd) {
    std::string out;
    FILE *fp = popen(cmd.c_str(), "r");
    if (!fp) return out;
    char buf[256];
    while (fgets(buf, sizeof(buf), fp)) out += buf;
    pclose(fp);
    return out;
}

static void print_pids(const std::string &pattern) {
    std::string cmd = "pgrep -x '" + pattern + "' 2>/dev/null || pgrep -f '" + pattern + "' 2>/dev/null || true";
    std::string out = run_cmd_capture(cmd);
    std::cout << "Processes matching '" << pattern << "': ";
    if (out.empty()) {
        std::cout << "(none)\n";
    } else {
        std::cout << out;
    }
}

static void start_background(const std::string &cmd) {
    std::string full = "setsid " + cmd + " >/dev/null 2>&1 &";
    system(full.c_str());
}

int main() {
    std::string procList = "xcalc,firefox";
    setenv("PROC_TO_KILL", procList.c_str(), 1);
    std::cout << "[USER] Set PROC_TO_KILL=" << procList << "\n";

    std::cout << "[USER] Starting calculator...\n";
    if (system("which xcalc >/dev/null 2>&1") == 0) {
        start_background("xcalc");
    } else {
        std::cout << "[USER] No known calculator found (xcalc/galculator/gnome-calculator). Install one or adjust procList.\n";
    }

    std::cout << "[USER] Starting firefox...\n";
    if (system("which firefox >/dev/null 2>&1") == 0) {
        start_background("firefox");
    } else {
        std::cout << "[USER] firefox not found in PATH\n";
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "\n[USER] Before running killer:\n";
    print_pids("xcalc");
    print_pids("firefox");

    std::string killerPath = "./killer";

    std::cout << "\n[USER] Running killer --name xcalc (or whichever calculator is present)\n";
    system((killerPath + " --name xcalc").c_str());
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n[USER] After killing calculator by name:\n";
    print_pids("xcalc");

    std::string out = run_cmd_capture("pgrep -x firefox 2>/dev/null || pgrep -f firefox 2>/dev/null || true");
    if (!out.empty()) {
        std::istringstream iss(out);
        int pid = 0;
        iss >> pid;
        if (pid > 0) {
            std::cout << "\n[USER] Killing firefox by pid " << pid << "\n";
            std::stringstream ss; ss << killerPath << " --id " << pid;
            system(ss.str().c_str());
        }
    } else {
        std::cout << "\n[USER] firefox PID not found to kill by id\n";
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "\n[USER] After killing firefox by id:\n";
    print_pids("firefox");

    std::cout << "\n[USER] Restarting calculator and firefox for PROC_TO_KILL test...\n";
    if (system("which xcalc >/dev/null 2>&1") == 0) start_background("xcalc");
    if (system("which firefox >/dev/null 2>&1") == 0) start_background("firefox");

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "\n[USER] Before PROC_TO_KILL processing:\n";
    print_pids("xcalc");
    print_pids("firefox");

    std::cout << "\n[USER] Running killer without args (reads PROC_TO_KILL)\n";
    system(killerPath.c_str());
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "\n[USER] After PROC_TO_KILL processing:\n";
    print_pids("xcalc");
    print_pids("firefox");

    std::cout << "\n[USER] Demo finished\n";
    return 0;
}
