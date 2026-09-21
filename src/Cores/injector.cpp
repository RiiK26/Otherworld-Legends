#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <string.h>

DWORD GetProcessIdByName(const char* procName)
{
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return 0;

  PROCESSENTRY32 pe;
  pe.dwSize = sizeof(pe);
  DWORD pid = 0;

  if (Process32First(hSnap, &pe)) {
    do {
      if (_stricmp(pe.szExeFile, procName) == 0) {
        pid = pe.th32ProcessID;
        break;
      }
    } while (Process32Next(hSnap, &pe));
  }

  CloseHandle(hSnap);
  return pid;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  int     argc;
  LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argvW)
    return 1;

  // Convert wide strings back to multibyte
  char procName[MAX_PATH];
  char dllPath[MAX_PATH];

  if (argc < 3) {
    LocalFree(argvW);
    return 1;
  }
  WideCharToMultiByte(CP_UTF8, 0, argvW[1], -1, procName, MAX_PATH, NULL, NULL);
  WideCharToMultiByte(CP_UTF8, 0, argvW[2], -1, dllPath, MAX_PATH, NULL, NULL);
  LocalFree(argvW);

  if (argc < 3) {
    return 1;
  }

  char fullDllPath[MAX_PATH];
  if (!GetFullPathNameA(dllPath, MAX_PATH, fullDllPath, nullptr)) {
    std::cerr << "Failed to resolve absolute path for DLL.\n";
    return 1;
  }

  DWORD fileAttr = GetFileAttributesA(fullDllPath);
  if (fileAttr == INVALID_FILE_ATTRIBUTES || (fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
    std::cerr << "DLL file not found or is a directory: " << fullDllPath << "\n";
    return 1;
  }

  DWORD pid = GetProcessIdByName(procName);

  if (pid == 0) {
    std::cerr << "Process not found: " << procName << "\n";
    return 1;
  }

  std::cout << "Found process " << procName << " with PID " << pid << "\n";

  HANDLE hProc = OpenProcess(
    PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
    FALSE, pid
  );
  if (!hProc) {
    std::cerr << "OpenProcess failed! Error: " << GetLastError() << "\n";
    return 1;
  }

  void* loc = VirtualAllocEx(hProc, 0, strlen(fullDllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!loc) {
    std::cerr << "VirtualAllocEx failed!\n";
    CloseHandle(hProc);
    return 1;
  }

  WriteProcessMemory(hProc, loc, fullDllPath, strlen(fullDllPath) + 1, 0);

  HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE) LoadLibraryA, loc, 0, 0);
  if (!hThread) {
    std::cerr << "CreateRemoteThread failed! Error: " << GetLastError() << "\n";
    VirtualFreeEx(hProc, loc, 0, MEM_RELEASE);
    CloseHandle(hProc);
    return 1;
  }

  WaitForSingleObject(hThread, INFINITE);
  VirtualFreeEx(hProc, loc, 0, MEM_RELEASE);
  CloseHandle(hThread);
  CloseHandle(hProc);

  std::cout << "Successfully injected " << fullDllPath << "\n";
  return 0;
}
