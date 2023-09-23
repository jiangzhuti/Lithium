/*
 * device_utils.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-29

Description: Device Utilities

**************************************************/

#include "device_utils.hpp"

#ifdef _WIN32
#include <windows.h>
#else

#endif
#include <iostream>
#include <array>
#include <memory>
#include <stdexcept>

#include <sstream>

std::string execute_command(const std::string &cmd)
{
    std::array<char, 128> buffer;
    std::string result = "";
    HANDLE pipeOutRead, pipeOutWrite;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;
    if (!CreatePipe(&pipeOutRead, &pipeOutWrite, &saAttr, 0))
    {
        throw std::runtime_error("Failed to create pipe!");
    }
    if (!SetHandleInformation(pipeOutRead, HANDLE_FLAG_INHERIT, 0))
    {
        throw std::runtime_error("Failed to set pipe handle information!");
    }
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = pipeOutWrite;
    si.hStdOutput = pipeOutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;
    if (!CreateProcess(NULL, const_cast<char *>(cmd.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        throw std::runtime_error("Failed to execute command!");
    }
    CloseHandle(pipeOutWrite);
    DWORD bytesRead;
    while (ReadFile(pipeOutRead, buffer.data(), buffer.size(), &bytesRead, NULL))
    {
        if (bytesRead == 0)
        {
            break;
        }
        result.append(buffer.data(), bytesRead);
    }
    CloseHandle(pipeOutRead);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return result;
}