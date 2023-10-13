/*
 * task_manager.cpp
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

Date: 2023-7-21

Description: Task Manager

**************************************************/

#include "task_manager.hpp"
#include "loguru/loguru.hpp"

using json = nlohmann::json;

namespace Lithium::Task
{
    TaskManager::TaskManager(const std::string &fileName)
        : m_FileName(fileName), m_StopFlag(false) {}

    bool TaskManager::addTask(const std::shared_ptr<BasicTask> &task)
    {
        if (!task)
        {
            DLOG_F(ERROR, "Invalid task!");
            return false;
        }
        m_TaskList.push_back(task);
        m_TaskMap[task->get_name()] = task;
        DLOG_F(INFO, "Task added: %s", task->get_name().c_str());
        return true;
    }

    bool TaskManager::insertTask(const std::shared_ptr<BasicTask> &task, int position)
    {
        if (!task)
        {
            DLOG_F(ERROR, "Error: Invalid task!");
            return false;
        }

        if (position < 0 || position >= m_TaskList.size())
        {
            DLOG_F(ERROR, "Error: Invalid position!");
            return false;
        }

        auto it = m_TaskList.begin() + position;
        m_TaskList.insert(it, task);
        DLOG_F(INFO, "Task inserted at position %d: %s", position, task->get_name().c_str());
        return true;
    }

    bool TaskManager::executeAllTasks()
    {
        for (auto it = m_TaskList.begin(); it != m_TaskList.end();)
        {
            auto &task = *it;
            if (!m_StopFlag && task)
            {
                try
                {
                    if (task->Execute())
                    {
                        DLOG_F(INFO, "Task executed: %s", task->get_name().c_str());
                        it = m_TaskList.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
                catch (const std::exception &ex)
                {
                    DLOG_F(ERROR, "Error: Failed to execute task %s - %s", task->get_name().c_str(), ex.what());
                    ++it;
                }
                catch (...)
                {
                    DLOG_F(ERROR, "Error: Failed to execute task %s", task->get_name().c_str());
                    ++it;
                }
            }
            else
            {
                ++it;
            }
        }
        return true;
    }

    void TaskManager::stopTask()
    {
        m_StopFlag = true;
    }

    bool TaskManager::executeTaskByName(const std::string &name)
    {
        auto it = findTaskByName(name);
        if (it != m_TaskMap.end() && !m_StopFlag && it->second)
        {
            try
            {
                if (it->second->Execute())
                {
                    DLOG_F(INFO, "Task executed: %s", it->second->get_name().c_str());
                }
                else
                {
                    DLOG_F(ERROR, "Error: Failed to execute task %s", it->second->get_name().c_str());
                }
                return true;
            }
            catch (const std::exception &ex)
            {
                DLOG_F(ERROR, "Error: Failed to execute task %s - %s", it->second->get_name().c_str(), ex.what());
            }
            catch (...)
            {
                DLOG_F(ERROR, "Error: Failed to execute task %s", it->second->get_name().c_str());
            }
        }
        else
        {
            DLOG_F(ERROR, "Error: Task not found or invalid!");
        }
        return false;
    }

    bool TaskManager::modifyTask(int index, const std::shared_ptr<BasicTask> &task)
    {
        if (!task)
        {
            DLOG_F(ERROR, "Error: Invalid task!");
            return false;
        }

        if (index < 0 || index >= m_TaskList.size())
        {
            DLOG_F(ERROR, "Error: Invalid index!");
            return false;
        }

        m_TaskList[index] = task;
        DLOG_F(INFO, "Task modified at index %d: %s", index, task->get_name().c_str());
        return true;
    }

    bool TaskManager::modifyTaskByName(const std::string &name, const std::shared_ptr<BasicTask> &task)
    {
        auto it = findTaskByName(name);
        if (it != m_TaskMap.end() && task)
        {
            it->second = task;
            DLOG_F(INFO, "Task modified : %s", task->get_name().c_str());
            return true;
        }
        return false;
    }

    bool TaskManager::deleteTask(int index)
    {
        if (index < 0 || index >= m_TaskList.size())
        {
            DLOG_F(ERROR, "Error: Invalid index!");
            return false;
        }

        auto it = m_TaskList.begin() + index;
        auto task = *it;
        m_TaskList.erase(it);
        DLOG_F(INFO, "Task deleted at index %d: %s", index, task->get_name().c_str());
        return true;
    }

    bool TaskManager::deleteTaskByName(const std::string &name)
    {
        auto it = findTaskByName(name);
        if (it != m_TaskMap.end())
        {
            auto task = it->second;
            m_TaskList.erase(std::remove(m_TaskList.begin(), m_TaskList.end(), task), m_TaskList.end());
            m_TaskMap.erase(it);
            DLOG_F(INFO, "Task deleted: %s", task->get_name().c_str());
            return true;
        }
        DLOG_F(ERROR, "Error: Task not found!");
        return false;
    }

    bool TaskManager::queryTaskByName(const std::string &name)
    {
        auto it = findTaskByName(name);
        if (it != m_TaskMap.end())
        {
            DLOG_F(INFO, "Task found: %s", it->second->get_name().c_str());
            return true;
        }
        else
        {
            DLOG_F(INFO, "Task not found!");
            return false;
        }
    }

    const std::vector<std::shared_ptr<BasicTask>> &TaskManager::getTaskList() const
    {
        return m_TaskList;
    }

    bool TaskManager::saveTasksToJson() const
    {
        json jsonArray;
        for (const auto &task : m_TaskList)
        {
            if (task)
            {
                json jsonObj;
                jsonObj["id"] = task->get_name();
                jsonObj["name"] = task->get_name();
                jsonObj["description"] = task->get_description();
                jsonArray.push_back(jsonObj);
            }
        }

        std::ofstream outputFile(m_FileName);
        if (!outputFile.is_open())
        {
            DLOG_F(ERROR, "Error: Failed to open file for writing!");
            return false;
        }

        outputFile << jsonArray.dump(4);
        outputFile.close();
        DLOG_F(INFO, "Tasks saved to JSON file: %s", m_FileName.c_str());
        return true;
    }

    std::unordered_map<std::string, std::shared_ptr<BasicTask>>::iterator TaskManager::findTaskByName(const std::string &name)
    {
        return std::find_if(m_TaskMap.begin(), m_TaskMap.end(),
                            [&](const std::pair<std::string, std::shared_ptr<BasicTask>> &item)
                            {
                                return item.second->get_name() == name;
                            });
    }
}