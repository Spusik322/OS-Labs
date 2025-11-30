#include <windows.h>
#include <iostream>
#include <string>

int main() {
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    
    HANDLE hPipeMA_r, hPipeMA_w;
    HANDLE hPipeAP_r, hPipeAP_w;
    HANDLE hPipePS_r, hPipePS_w;

    CreatePipe(&hPipeMA_r, &hPipeMA_w, &sa, 0);
    CreatePipe(&hPipeAP_r, &hPipeAP_w, &sa, 0);
    CreatePipe(&hPipePS_r, &hPipePS_w, &sa, 0);

    STARTUPINFO siM = {sizeof(STARTUPINFO)};
    STARTUPINFO siA = {sizeof(STARTUPINFO)};
    STARTUPINFO siP = {sizeof(STARTUPINFO)};
    STARTUPINFO siS = {sizeof(STARTUPINFO)};
    
    PROCESS_INFORMATION piM, piA, piP, piS;
    
    siM.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siM.hStdOutput = hPipeMA_w;
    siM.dwFlags = STARTF_USESTDHANDLES;
    
    siA.hStdInput = hPipeMA_r;
    siA.hStdOutput = hPipeAP_w;
    siA.dwFlags = STARTF_USESTDHANDLES;
    
    siP.hStdInput = hPipeAP_r;
    siP.hStdOutput = hPipePS_w;
    siP.dwFlags = STARTF_USESTDHANDLES;
    
    siS.hStdInput = hPipePS_r;
    siS.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siS.dwFlags = STARTF_USESTDHANDLES;
    
    CreateProcess(NULL, "M.exe", NULL, NULL, TRUE, 0, NULL, NULL, &siM, &piM);
    CreateProcess(NULL, "A.exe", NULL, NULL, TRUE, 0, NULL, NULL, &siA, &piA);
    CreateProcess(NULL, "P.exe", NULL, NULL, TRUE, 0, NULL, NULL, &siP, &piP);
    CreateProcess(NULL, "S.exe", NULL, NULL, TRUE, 0, NULL, NULL, &siS, &piS);

    WaitForSingleObject(piM.hProcess, INFINITE);
    WaitForSingleObject(piA.hProcess, INFINITE);
    WaitForSingleObject(piP.hProcess, INFINITE);
    WaitForSingleObject(piS.hProcess, INFINITE);

    CloseHandle(piM.hProcess);
    CloseHandle(piA.hProcess);
    CloseHandle(piP.hProcess);
    CloseHandle(piS.hProcess);
    
    CloseHandle(hPipeMA_r);
    CloseHandle(hPipeMA_w);
    CloseHandle(hPipeAP_r);
    CloseHandle(hPipeAP_w);
    CloseHandle(hPipePS_r);
    CloseHandle(hPipePS_w);
    
    return 0;
}