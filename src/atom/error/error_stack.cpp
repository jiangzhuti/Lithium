/*
 * error_stack.cpp
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

Description: Error Stack

**************************************************/

#include "error_stack.hpp"

#include <ctime>
#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/utils/time.hpp"

std::ostream &operator<<(std::ostream &os, const ErrorInfo &error)
{
    os << "{"
       << "\"errorMessage\": \"" << error.errorMessage << "\","
       << "\"moduleName\": \"" << error.moduleName << "\","
       << "\"functionName\": \"" << error.functionName << "\","
       << "\"line\": " << error.line << ","
       << "\"fileName\": \"" << error.fileName << "\","
       << "\"timestamp\": \"" << TimeStampToString(error.timestamp) << "\","
       << "\"uuid\": \"" << error.uuid << "\""
       << "}";

    return os;
}

std::string operator<<(const std::string &str, const ErrorInfo &error)
{
    std::stringstream ss;
    ss << "{"
       << "\"errorMessage\": \"" << error.errorMessage << "\","
       << "\"moduleName\": \"" << error.moduleName << "\","
       << "\"functionName\": \"" << error.functionName << "\","
       << "\"line\": " << error.line << ","
       << "\"fileName\": \"" << error.fileName << "\","
       << "\"timestamp\": \"" << TimeStampToString(error.timestamp) << "\","
       << "\"uuid\": \"" << error.uuid << "\""
       << "}";

    return ss.str();
}

ErrorStack::ErrorStack()
{
}

std::shared_ptr<ErrorStack> ErrorStack::createShared()
{
    return std::make_shared<ErrorStack>();
}

std::unique_ptr<ErrorStack> ErrorStack::createUnique()
{
    return std::make_unique<ErrorStack>();
}

void ErrorStack::InsertError(const std::string &errorMessage, const std::string &moduleName, const std::string &functionName, int line, const std::string &fileName)
{
    ErrorInfo error;
    error.errorMessage = errorMessage;
    error.moduleName = moduleName;
    error.timestamp = std::time(nullptr);
    error.functionName = functionName;
    error.line = line;
    error.fileName = fileName;

    errorStack.push_back(error);
}

void ErrorStack::SetFilteredModules(const std::vector<std::string> &modules)
{
    filteredModules = modules;
}

void ErrorStack::ClearFilteredModules()
{
    filteredModules.clear();
}

void ErrorStack::PrintFilteredErrorStack() const
{
    for (const auto &error : errorStack)
    {
        if (std::find(filteredModules.begin(), filteredModules.end(), error.moduleName) == filteredModules.end())
        {
            LOG_F(ERROR, "%s", error.errorMessage.c_str());
        }
    }
}

std::vector<ErrorInfo> ErrorStack::GetFilteredErrorsByModule(const std::string &moduleName) const
{
    std::vector<ErrorInfo> errors;

    std::copy_if(errorStack.begin(), errorStack.end(), std::back_inserter(errors),
                 [&moduleName, this](const ErrorInfo &error)
                 {
                     return error.moduleName == moduleName && std::find(filteredModules.begin(), filteredModules.end(), error.moduleName) == filteredModules.end();
                 });

    return errors;
}

void ErrorStack::InsertErrorCompressed(const std::string &errorMessage, const std::string &moduleName, const std::string &functionName, int line, const std::string &fileName)
{
    time_t currentTime = std::time(nullptr);

    auto iter = std::find_if(errorStack.begin(), errorStack.end(),
                             [&errorMessage, &moduleName](const ErrorInfo &error)
                             {
                                 return error.errorMessage == errorMessage && error.moduleName == moduleName;
                             });

    if (iter != errorStack.end())
    {
        iter->timestamp = currentTime;
    }
    else
    {
        ErrorInfo error;
        error.errorMessage = errorMessage;
        error.moduleName = moduleName;
        error.timestamp = currentTime;
        error.functionName = functionName;
        error.line = line;
        error.fileName = fileName;

        errorStack.push_back(error);
    }

    UpdateCompressedErrors();
}

std::string ErrorStack::GetCompressedErrors() const
{
    std::string compressedErrors;

    for (const auto &error : compressedErrorStack)
    {
        compressedErrors << error;
    }

    return compressedErrors;
}

void ErrorStack::UpdateCompressedErrors()
{
    compressedErrorStack.clear();

    for (const auto &error : errorStack)
    {
        auto iter = std::find_if(compressedErrorStack.begin(), compressedErrorStack.end(),
                                 [&error](const ErrorInfo &compressedError)
                                 {
                                     return compressedError.errorMessage == error.errorMessage && compressedError.moduleName == error.moduleName;
                                 });

        if (iter != compressedErrorStack.end())
        {
            iter->timestamp = error.timestamp;
        }
        else
        {
            compressedErrorStack.push_back(error);
        }
    }

    SortCompressedErrorStack();
}

void ErrorStack::SortCompressedErrorStack()
{
    std::sort(compressedErrorStack.begin(), compressedErrorStack.end(),
              [](const ErrorInfo &error1, const ErrorInfo &error2)
              {
                  return error1.timestamp > error2.timestamp;
              });
}
