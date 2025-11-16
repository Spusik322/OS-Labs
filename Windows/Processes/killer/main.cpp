#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

static std::wstring ToLower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), ::towlower);
    return s;
}

static std::vector<std::wstring> SplitCommaList(const std::wstring &s) {
    std::vector<std::wstring> out;
    std::wstringstream ss(s);
    std::wstring item;
    while (std::getline(ss, item, L',')) {
        size_t a = item.find_first_not_of(L" \t\r\n\"");
        size_t b = item.find_last_not_of(L" \t\r\n\"");
        if (a == std::wstring::npos) continue;
        out.push_back(item.substr(a, b - a + 1));
    }
    return out;
}

static std::vector<DWORD> FindProcessesByName(const std::wstring &name) {
    std::vector<DWORD> pids;
    std::wstring target = ToLower(name);
    std::wstring targetWithExe = target;
    if (targetWithExe.size() < 4 || targetWithExe.substr(targetWithExe.size() - 4) != L".exe")
        targetWithExe += L".exe";

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        std::wcerr << L"[KILL] CreateToolhelp32Snapshot failed (err=" << GetLastError() << L")\n";
        return pids;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            std::wstring exe = ToLower(pe.szExeFile);
            if (exe == target || exe == targetWithExe) {
                pids.push_back(pe.th32ProcessID);
            }
        } while (Process32NextW(snap, &pe));
    } else {
        std::wcerr << L"[KILL] Process32FirstW failed (err=" << GetLastError() << L")\n";
    }
    CloseHandle(snap);
    return pids;
}

static bool KillProcessById(DWORD pid) {
    if (pid == 0) return false;
    
    HANDLE h = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            std::wcerr << L"[KILL] Access denied to PID " << pid << L". Try running as administrator.\n";
        } else if (err == ERROR_INVALID_PARAMETER) {
            std::wcout << L"[KILL] Process " << pid << L" does not exist\n";
        } else {
            std::wcerr << L"[KILL] OpenProcess failed for PID " << pid << L" (err=" << err << L")\n";
        }
        return false;
    }

    DWORD exitCode;
    if (!GetExitCodeProcess(h, &exitCode)) {
        std::wcerr << L"[KILL] GetExitCodeProcess failed for PID " << pid << L" (err=" << GetLastError() << L")\n";
        CloseHandle(h);
        return false;
    }

    if (exitCode != STILL_ACTIVE) {
        std::wcout << L"[KILL] Process " << pid << L" is already terminated\n";
        CloseHandle(h);
        return true;
    }

    if (TerminateProcess(h, 0)) {
        std::wcout << L"[KILL] Successfully terminated PID " << pid << L"\n";
        CloseHandle(h);
        return true;
    } else {
        DWORD err = GetLastError();
        std::wcerr << L"[KILL] TerminateProcess failed for PID " << pid << L" (err=" << err << L")\n";
        CloseHandle(h);
        return false;
    }
}

static void KillByName(const std::wstring &name) {
    std::wcout << L"[KILL] Searching for processes named: " << name << L"\n";
    auto pids = FindProcessesByName(name);
    if (pids.empty()) {
        std::wcout << L"[KILL] No processes found by name: " << name << L"\n";
        return;
    }
    
    std::wcout << L"[KILL] Found " << pids.size() << L" process(es) with name: " << name << L"\n";
    
    std::sort(pids.begin(), pids.end());
    pids.erase(std::unique(pids.begin(), pids.end()), pids.end());
    
    for (DWORD pid : pids) {
        if (pid != 0 && pid != GetCurrentProcessId()) {
            KillProcessById(pid);
        }
    }
}

static void PrintUsage() {
    std::wcout << L"Usage: killer [--id <pid>] [--name <process_name>]\n";
    std::wcout << L"Also reads PROC_TO_KILL environment variable (comma-separated names).\n";
}

int wmain(int argc, wchar_t** argv) {
    bool processedArgs = false;

    for (int i = 1; i < argc; ++i) {
        std::wstring arg = argv[i];
        if (arg == L"--id") {
            if (i + 1 >= argc) { 
                PrintUsage(); 
                return 1; 
            }
            DWORD pid = (DWORD)_wtoi(argv[++i]);
            if (pid == 0) {
                std::wcerr << L"[KILL] Invalid PID value: " << argv[i] << L"\n";
            } else {
                KillProcessById(pid);
                processedArgs = true;
            }
        } else if (arg == L"--name") {
            if (i + 1 >= argc) { 
                PrintUsage(); 
                return 1; 
            }
            KillByName(argv[++i]);
            processedArgs = true;
        } else if (arg == L"--help" || arg == L"-h") {
            PrintUsage();
            return 0;
        } else {
            std::wcerr << L"[KILL] Unknown argument: " << arg << L"\n";
            PrintUsage();
            return 1;
        }
    }

    if (!processedArgs) {
        DWORD needed = GetEnvironmentVariableW(L"PROC_TO_KILL", NULL, 0);
        if (needed > 0) {
            std::wstring buf;
            buf.resize(needed);
            DWORD got = GetEnvironmentVariableW(L"PROC_TO_KILL", &buf[0], needed);
            if (got > 0 && got < needed) {
                buf.resize(got);
                auto list = SplitCommaList(buf);
                if (!list.empty()) {
                    std::wcout << L"[KILL] Processing PROC_TO_KILL: " << buf << L"\n";
                    for (auto &name : list) {
                        if (!name.empty()) {
                            KillByName(name);
                        }
                    }
                }
            }
        }
    }

    return 0;
}