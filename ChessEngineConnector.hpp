#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

STARTUPINFO sti = {0};
SECURITY_ATTRIBUTES sats = {0};
PROCESS_INFORMATION pi = {0};
HANDLE pipin_w, pipin_r, pipout_w, pipout_r;
BYTE buffer[2048];
DWORD writ, excode, read, available;

void ConnectToEngine(const char* path) {
    pipin_w = pipin_r = pipout_w = pipout_r = NULL;
    ZeroMemory(&sats, sizeof(sats));
    sats.nLength = sizeof(SECURITY_ATTRIBUTES);
    sats.bInheritHandle = TRUE;
    sats.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&pipout_r, &pipout_w, &sats, 0)) {
        std::cerr << "Failed to create output pipe, error: " << GetLastError() << std::endl;
        return;
    }

    if (!CreatePipe(&pipin_r, &pipin_w, &sats, 0)) {
        std::cerr << "Failed to create input pipe, error: " << GetLastError() << std::endl;
        CloseHandle(pipout_r);
        CloseHandle(pipout_w);
        return;
    }

    ZeroMemory(&sti, sizeof(sti));
    sti.cb = sizeof(STARTUPINFO);
    sti.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    sti.wShowWindow = SW_HIDE;
    sti.hStdInput = pipin_r;
    sti.hStdOutput = pipout_w;
    sti.hStdError = pipout_w;

    ZeroMemory(&pi, sizeof(pi));

    std::vector<char> commandLine(path, path + strlen(path) + 1);

    if (!CreateProcess(NULL, commandLine.data(), NULL, NULL, TRUE, 0, NULL, NULL, &sti, &pi)) {
        std::cerr << "Failed to start process, error: " << GetLastError() << std::endl;
        CloseHandle(pipout_r);
        CloseHandle(pipout_w);
        CloseHandle(pipin_r);
        CloseHandle(pipin_w);
        return;
    }
}

std::string getNextMove(std::string position)
{
    std::string str;
    position = "position startpos moves "+position+"\ngo\n";

    WriteFile(pipin_w, position.c_str(), position.length(),&writ, NULL);
    Sleep(500);

    PeekNamedPipe(pipout_r, buffer,sizeof(buffer), &read, &available, NULL);
    do
    {
        ZeroMemory(buffer, sizeof(buffer));
        if(!ReadFile(pipout_r, buffer, sizeof(buffer), &read, NULL) || !read) break;
        buffer[read] = 0;
        str+=(char*)buffer;
    }
    while(read >= sizeof(buffer));

    int n = str.find("bestmove");
    if (n!=-1) return str.substr(n+9,4);

    return "error";
}


void CloseConnection()
{
    WriteFile(pipin_w, "quit\n", 5,&writ, NULL);
    if(pipin_w != NULL) CloseHandle(pipin_w);
    if(pipin_r != NULL) CloseHandle(pipin_r);
    if(pipout_w != NULL) CloseHandle(pipout_w);
    if(pipout_r != NULL) CloseHandle(pipout_r);
    if(pi.hProcess != NULL) CloseHandle(pi.hProcess);
    if(pi.hThread != NULL) CloseHandle(pi.hThread);
}


#endif //CONNECTOR_H
