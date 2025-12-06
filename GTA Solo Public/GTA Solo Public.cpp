#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

#include <iostream>
#include <thread>
#include <vector>

using namespace std;

int delay = 10;

auto GetProcessId(PCSTR name) -> DWORD
{
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    ZeroMemory(&process, sizeof(process));
    process.dwSize = sizeof(process);
    if (Process32First(snapshot, &process))
    {
        do
        {
            if (string(process.szExeFile) == string(name))
            {
                pid = process.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &process));
    }
    CloseHandle(snapshot);
    return pid;
}

auto GetProcessThreads(DWORD pid) -> std::vector<DWORD> 
{
    std::vector<DWORD> threads;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (snapshot == INVALID_HANDLE_VALUE) return threads;

    THREADENTRY32 te;
    te.dwSize = sizeof(te);

    if (Thread32First(snapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid)
                threads.push_back(te.th32ThreadID);
        } while (Thread32Next(snapshot, &te));
    }

    CloseHandle(snapshot);
    return threads;
}


auto SuspendProcess(DWORD pid) -> void
{
    for (DWORD tid : GetProcessThreads(pid)) {
        HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, tid);
        if (thread) {
            SuspendThread(thread);
            CloseHandle(thread);
        }
    }
}

auto ResumeProcess(DWORD pid) -> void
{
    for (DWORD tid : GetProcessThreads(pid)) {
        HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, tid);
        if (thread) {
            while (ResumeThread(thread) > 0);
            CloseHandle(thread);
        }
    }
}

auto main(int ac, char** argv) -> int
{
    DWORD id = GetProcessId("GTA5_Enhanced.exe");

    if (!id)
    {
        cout << "Couldn't find GTA5_Enhanced.exe!\n";
        return 0;
    }
    
    SuspendProcess(id);
    cout << "Process suspended, waiting " << delay << " seconds for timeout...\n";
    for (size_t i = 1; i < delay; i++)
    {
        cout << "Time left until resuming: " << (delay - i) << " seconds...\n";
        this_thread::sleep_for(chrono::seconds(1));
    }
    ResumeProcess(id);
    cout << "Process resumed, you should now be in a solo public!\n";
}