#pragma once

#include "Window/WindowsEx.h"
#include "UICore/Helper/StringHelper.h"

#include <string>
#include <sstream>
#include <vector>
#include <thread>

class ProcessRunner
{
    STARTUPINFO _si;
    PROCESS_INFORMATION _pi;
    HANDLE _hChildStdinRead = NULL;
    HANDLE _hChildStdinWrite = NULL;
    HANDLE _hChildStdoutRead = NULL;
    HANDLE _hChildStdoutWrite = NULL;
    DWORD _returnCode = -1;
    bool _finished = true;
    std::string _output = "";

    std::wstring _filename;

    std::thread _readerThread;
public:
    ProcessRunner(std::wstring filename)
    {
        _filename = filename;
    }
    ProcessRunner(std::string filename)
    {
        _filename = string_to_wstring(filename);
    }
    ~ProcessRunner()
    {
        Terminate();
        while (!Finished()) {}
    }

    bool StartProcess()
    {
        ZeroMemory(&_si, sizeof(_si));
        _si.cb = sizeof(_si);
        ZeroMemory(&_pi, sizeof(_pi));

        // Create pipes
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        // Create a pipe for the child process's STDOUT. 
        if (!CreatePipe(&_hChildStdoutRead, &_hChildStdoutWrite, &saAttr, 0))
        {
            return false;
        }
        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (!SetHandleInformation(_hChildStdoutRead, HANDLE_FLAG_INHERIT, 0))
        {
            return false;
        }
        // Create a pipe for the child process's STDIN. 
        if (!CreatePipe(&_hChildStdinRead, &_hChildStdinWrite, &saAttr, 0))
        {
            return false;
        }
        // Ensure the write handle to the pipe for STDIN is not inherited. 
        if (!SetHandleInformation(_hChildStdinWrite, HANDLE_FLAG_INHERIT, 0))
        {
            return false;
        }
        ZeroMemory(&_si, sizeof(STARTUPINFO));
        _si.cb = sizeof(STARTUPINFO);
        _si.hStdError = _hChildStdoutWrite;
        _si.hStdOutput = _hChildStdoutWrite;
        _si.hStdInput = _hChildStdinRead;
        _si.dwFlags |= STARTF_USESTDHANDLES;

        // Start the child process. 
        if (!CreateProcess(NULL,   // No module name (use command line)
            &_filename[0],        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            TRUE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &_si,            // Pointer to STARTUPINFO structure
            &_pi)           // Pointer to PROCESS_INFORMATION structure
            )
        {
            printf("CreateProcess failed (%d)\n", GetLastError());
            return false;
        }
        else
        {
            CloseHandle(_hChildStdoutWrite);
            CloseHandle(_hChildStdinRead);
        }
        _output = "";
        _finished = false;

        _readerThread = std::thread(&ProcessRunner::ReadFromProcess, this);
        _readerThread.detach();

        return true;
    }

    bool Finished()
    {
        if (!_finished)
        {
            // Wait until child process exits.
            if (WaitForSingleObject(_pi.hProcess, 0) == WAIT_OBJECT_0)
            {
                // Get exit code
                GetExitCodeProcess(_pi.hProcess, &_returnCode);

                // Close process and thread handles. 
                CloseHandle(_pi.hProcess);
                CloseHandle(_pi.hThread);

                _finished = true;
                return true;
            }
            return false;
        }
        return true;
    }

    void Terminate()
    {
        if (!_finished)
        {
            TerminateProcess(_pi.hProcess, -1);
        }
    }

    int ExitCode()
    {
        return _returnCode;
    }

    bool WriteToProcess(const std::string& str, unsigned long* written)
    {
        BOOL success = WriteFile(_hChildStdinWrite, &str[0], str.length(), written, NULL);
        if (!success)
            printf("WriteFile failed (%d)\n", GetLastError());
        return success;
    }

    void ReadFromProcess()
    {
        while (true)
        {
            CHAR buf[4096];
            DWORD read = 0;
            BOOL success = ReadFile(_hChildStdoutRead, buf, 4096, &read, NULL);
            if (!success || read == 0)
            {
                printf("ReadFile failed (%d)\n", GetLastError());
                break;
            }
            _output += std::string(&buf[0], &buf[read]);
        }
    }

    std::string GetOutput()
    {
        return _output;
    }
};