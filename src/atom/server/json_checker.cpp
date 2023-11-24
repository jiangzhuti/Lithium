/*
 * json_checker.hpp
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

Date: 2023-11-11

Description: Json Checker interface implementation

**************************************************/

#include "json_checker.hpp"

JsonChecker::JsonChecker()
{
    // 初始化默认规则
    addDefaultRule<std::string>("string", [](const json &jsonData)
                                { return jsonData.is_string(); });

    addDefaultRule<int>("integer", [](const json &jsonData)
                        { return jsonData.is_number_integer(); });

    addDefaultRule<double>("number", [](const json &jsonData)
                           { return jsonData.is_number(); });

    addDefaultRule<bool>("boolean", [](const json &jsonData)
                         { return jsonData.is_boolean(); });
}

void JsonChecker::onFailure(const std::string &message)
{
    if (failureCallback_)
    {
        failureCallback_(message);
    }
}

void JsonChecker::setFailureCallback(std::function<void(const std::string &)> callback)
{
    failureCallback_ = std::move(callback);
}

/*

int main()
{
    json templateData = R"(
        {
            "name": "string",
            "age": 0,
            "isStudent": false,
            "email": "string"
        }
    )"_json;

    json jsonData = R"(
        {
            "name": "Alice",
            "age": 25,
            "isStudent": true,
            "email": "alice@example.com"
        }
    )"_json;

    JsonChecker checker;
    checker.setFailureCallback([](const std::string &message)
                               { std::cerr << "JSON 数据与模板不匹配: " << message << std::endl; });

    // 添加自定义规则
    checker.addCustomRule<bool>("boolean", [](const json &jsonData)
                                { return jsonData.is_boolean() || jsonData.is_string(); });

    bool isMatched = true;
    isMatched &= checker.checkType<std::string>(jsonData["name"], templateData["name"]);
    isMatched &= checker.checkType<int>(jsonData["age"], templateData["age"]);
    isMatched &= checker.checkType<bool>(jsonData["isStudent"], templateData["isStudent"]);
    isMatched &= checker.validateFormat<std::string>(jsonData["email"], R"(\w+@\w+\.\w+)");

    if (isMatched)
    {
        std::cout << "JSON 数据与模板匹配！" << std::endl;
    }

    return 0;
}

*/