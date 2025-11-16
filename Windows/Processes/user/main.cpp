#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

static std::vector<DWORD> FindProcessesByName(const std::wstring &name) {
    std::vector<DWORD> pids;
    std::wstring target = name;
    std::transform(target.begin(), target.end(), target.begin(), ::towlower);
    
    std::wstring targetWithExe = target;
    if (targetWithExe.size() < 4 || targetWithExe.substr(targetWithExe.size() - 4) != L".exe")
        targetWithExe += L".exe";

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return pids;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            std::wstring exe = pe.szExeFile;
            std::transform(exe.begin(), exe.end(), exe.begin(), ::towlower);
            if (exe == target || exe == targetWithExe) {
                pids.push_back(pe.th32ProcessID);
            }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pids;
}

static void PrintProcessesNamed(const std::wstring &name) {
    auto v = FindProcessesByName(name);
    std::wcout << L"Processes named '" << name << L"': ";
    if (v.empty()) {
        std::wcout << L"(none)\n";
    } else {
        for (DWORD pid : v) std::wcout << pid << L" ";
        std::wcout << L"\n";
    }
}

static bool StartProcess(const std::wstring &cmdline, PROCESS_INFORMATION &outPi) {
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&outPi, sizeof(outPi));

    std::wstring cmd = cmdline;
    if (!CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &outPi)) {
        std::wcerr << L"[USER] CreateProcessW failed for '" << cmdline << L"' (err=" << GetLastError() << L")\n";
        return false;
    }
    
    Sleep(2000);
    return true;
}

static bool RunKiller(const std::wstring &killerPath, const std::wstring &args) {
    std::wstring cmd = L"\"" + killerPath + L"\" " + args;
    std::wcout << L"[USER] Running: " << cmd << L"\n";

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring cmdcopy = cmd;
    if (!CreateProcessW(NULL, &cmdcopy[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::wcerr << L"[USER] Failed to start killer (err=" << GetLastError() << L")\n";
        return false;
    }
    
    WaitForSingleObject(pi.hProcess, 10000);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

int wmain() {
    std::wstring procList = L"notepad.exe,CalculatorApp.exe";
    if (!SetEnvironmentVariableW(L"PROC_TO_KILL", procList.c_str())) {
        std::wcerr << L"[USER] SetEnvironmentVariableW failed (err=" << GetLastError() << L")\n";
    } else {
        std::wcout << L"[USER] Set PROC_TO_KILL=" << procList << L"\n";
    }

    PROCESS_INFORMATION piNotepad = {0}, piCalc = {0};
    bool okNotepad = StartProcess(L"notepad.exe", piNotepad);
    bool okCalc = StartProcess(L"calc.exe", piCalc);

    if (!okNotepad && !okCalc) {
        std::wcerr << L"[USER] Failed to start test processes\n";
        return 1;
    }

    std::wcout << L"\n[USER] Before running killer:\n";
    PrintProcessesNamed(L"notepad.exe");
    PrintProcessesNamed(L"CalculatorApp.exe");

    wchar_t selfPath[MAX_PATH];
    GetModuleFileNameW(NULL, selfPath, MAX_PATH);
    std::wstring self(selfPath);
    size_t pos = self.find_last_of(L"\\/");
    std::wstring folder = (pos == std::wstring::npos) ? L"." : self.substr(0, pos);
    std::wstring killerPath = folder + L"\\killer.exe";

    std::wcout << L"\n[USER] Testing --name parameter for notepad...\n";
    RunKiller(killerPath, L"--name notepad.exe");
    Sleep(1000);
    
    std::wcout << L"\n[USER] After killing notepad:\n";
    PrintProcessesNamed(L"notepad.exe");

    std::wcout << L"\n[USER] Testing --id parameter for calculator...\n";
    PROCESS_INFORMATION piCalcNew = {0};
    StartProcess(L"calc.exe", piCalcNew);
    Sleep(2000);
    
    DWORD calcPid = 0;
    auto calcProcesses = FindProcessesByName(L"CalculatorApp.exe");
    if (!calcProcesses.empty()) {
        calcPid = calcProcesses.front();
        std::wstringstream ss;
        ss << L"--id " << calcPid;
        RunKiller(killerPath, ss.str());
    } else {
        std::wcout << L"[USER] CalculatorApp.exe not found\n";
    }

    Sleep(1000);
    std::wcout << L"\n[USER] After killing calculator:\n";
    PrintProcessesNamed(L"CalculatorApp.exe");

    std::wcout << L"\n[USER] Restarting processes for PROC_TO_KILL test...\n";
    PROCESS_INFORMATION piN2 = {0}, piC2 = {0};
    StartProcess(L"notepad.exe", piN2);
    StartProcess(L"calc.exe", piC2);
    
    Sleep(2000);
    std::wcout << L"\n[USER] Before PROC_TO_KILL processing:\n";
    PrintProcessesNamed(L"notepad.exe");
    PrintProcessesNamed(L"CalculatorApp.exe");

    std::wcout << L"\n[USER] Testing PROC_TO_KILL environment variable...\n";
    RunKiller(killerPath, L"");
    
    Sleep(1000);
    std::wcout << L"\n[USER] After PROC_TO_KILL processing:\n";
    PrintProcessesNamed(L"notepad.exe");
    PrintProcessesNamed(L"CalculatorApp.exe");

    if (piNotepad.hProcess) { 
        TerminateProcess(piNotepad.hProcess, 0);
        CloseHandle(piNotepad.hProcess); 
        CloseHandle(piNotepad.hThread); 
    }
    if (piCalc.hProcess) { 
        TerminateProcess(piCalc.hProcess, 0);
        CloseHandle(piCalc.hProcess); 
        CloseHandle(piCalc.hThread); 
    }
    if (piCalcNew.hProcess) { 
        TerminateProcess(piCalcNew.hProcess, 0);
        CloseHandle(piCalcNew.hProcess); 
        CloseHandle(piCalcNew.hThread); 
    }
    if (piN2.hProcess) { 
        TerminateProcess(piN2.hProcess, 0);
        CloseHandle(piN2.hProcess); 
        CloseHandle(piN2.hThread); 
    }
    if (piC2.hProcess) { 
        TerminateProcess(piC2.hProcess, 0);
        CloseHandle(piC2.hProcess); 
        CloseHandle(piC2.hThread); 
    }

    std::wcout << L"\n[USER] Demo finished.\n";
    return 0;
}