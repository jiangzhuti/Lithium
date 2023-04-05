/*
 * openapt.cpp
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
 
Date: 2023-3-27
 
Description: Main 
 
**************************************************/

/*

   ____                   ___    ____  ______
  / __ \____  ___  ____  /   |  / __ \/_  __/
 / / / / __ \/ _ \/ __ \/ /| | / /_/ / / /   
/ /_/ / /_/ /  __/ / / / ___ |/ ____/ / /    
\____/ .___/\___/_/ /_/_/  |_/_/     /_/     
    /_/                                      

    __  ___      __           ___         __                   __          __                               __             ______           _ __     
   /  |/  /___ _/ /_____     /   |  _____/ /__________  ____  / /_  ____  / /_____  ____ __________ _____  / /_  __  __   / ____/___ ______(_) /_  __
  / /|_/ / __ `/ //_/ _ \   / /| | / ___/ __/ ___/ __ \/ __ \/ __ \/ __ \/ __/ __ \/ __ `/ ___/ __ `/ __ \/ __ \/ / / /  / __/ / __ `/ ___/ / / / / /
 / /  / / /_/ / ,< /  __/  / ___ |(__  ) /_/ /  / /_/ / /_/ / / / / /_/ / /_/ /_/ / /_/ / /  / /_/ / /_/ / / / / /_/ /  / /___/ /_/ (__  ) / / /_/ / 
/_/  /_/\__,_/_/|_|\___/  /_/  |_/____/\__/_/   \____/ .___/_/ /_/\____/\__/\____/\__, /_/   \__,_/ .___/_/ /_/\__, /  /_____/\__,_/____/_/_/\__, /  
                                                    /_/                          /____/          /_/          /____/                        /____/   
*/

#include "openapt.hpp"
#include "plugins/crash.hpp"

#include <spdlog/spdlog.h> // 引入 spdlog 日志库

crow::SimpleApp app;

#include <fstream>
#include <vector>
#include <memory>
#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <cstdlib>
#include <functional>
#include <exception>
#include <stdexcept>
#include <unistd.h>
#include <getopt.h>

#ifdef _WIN32
#include <winsock2.h> // Windows socket API
#include <ws2tcpip.h>
#include <windows.h>
#include <psapi.h>
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <dirent.h>
#endif

#include "nlohmann/json.hpp"

#include "device/basic_device.hpp"
#include "task/define.hpp"

using json = nlohmann::json;

OpenAPT::ThreadManager m_ThreadManager;
OpenAPT::TaskManager m_TaskManager;
OpenAPT::DeviceManager m_DeviceManager;
OpenAPT::ModuleLoader m_ModuleLoader;
OpenAPT::ConfigManager m_ConfigManager;

bool DEBUG = true;

void print_help(int argc, char* argv[]) {
    std::cout << "Usage: " << argv[0] << " [-d|--debug] [-p|--port PORT] [-c|--config CONFIG_FILE]\n"
              << "Options:\n"
              << "  -d, --debug                   Enable debug mode\n"
              << "  -p, --port PORT               Specify listening port (default is 8080)\n"
              << "  -c, --config CONFIG_FILE      Use custom config file (default is config.json)\n";
    exit(EXIT_SUCCESS);
}

void parse_args(int argc, char* argv[]) {
    int opt; // 存储选项字符值
    int option_index = 0; // 存储选项索引值
    const char* short_opts = "dp:c:"; // 定义需要解析的单字符选项
    const option long_opts[] = { // 定义需要解析的长选项
        {"debug", no_argument, nullptr, 'd'},
        {"port", required_argument, nullptr, 'p'},
        {"config", required_argument, nullptr, 'c'},
        {nullptr, 0, nullptr, 0}
    };

    while ((opt = getopt_long(argc, argv, short_opts, long_opts, &option_index)) != -1) { // 解析选项
        switch (opt) {
            case 'd':
                DEBUG = true;
                spdlog::info("DEBUG Mode is enabled by command line argument");
                break;
            case 'p':
                break;
            case 'c':
                break;
            case '?': // 处理未知选项
                print_help(argc,argv); // 抛出异常并在主函数中处理
                break;
            default:
                break;
        }
    }

    // 类似处理短选项的方式，处理剩余的非选项参数

    if (optind < argc) {
        // 处理剩余的非选项参数
    }
}

void quit();

#ifdef _WIN32
// Define the signal handler function for Windows platform
BOOL WINAPI interruptHandler(DWORD signalNumber) {
    if (signalNumber == CTRL_C_EVENT) {
        spdlog::info("Keyboard interrupt received.");
    }
    return TRUE;
}

// Register the signal handler function to deal with Ctrl+C on Windows platform
void registerInterruptHandler() {
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)interruptHandler, TRUE);
}
#else
// Define the signal handler function
void interruptHandler(int signalNumber, siginfo_t *info, void *context) {
    spdlog::info("Keyboard interrupt received.");
    quit();
}

// Register the signal handler function to deal with SIGINT on all platforms
void registerInterruptHandler() {
    struct sigaction sa;
    sa.sa_sigaction = &interruptHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaction(SIGINT, &sa, NULL);
}
#endif

/**
 @brief 检查指定端口是否被占用，并杀死占用该端口的进程
 @param port 端口号
 @return true 端口未被占用，或者已经成功杀死占用该端口的进程
 @return false 端口已被占用，但无法杀死占用该端口的进程
 */
bool CheckAndKillProgramOnPort(int port) {
#ifdef _WIN32
    // 初始化 Windows socket API
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        spdlog::error("Failed to initialize Windows Socket API: {}", ret);
        return false;
    }

    // 创建一个新的套接字
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        spdlog::error("Failed to create socket: {}", WSAGetLastError());
        WSACleanup();
        return false;
    }

    // 绑定到指定端口上
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        if (WSAGetLastError() == WSAEADDRINUSE) {
            spdlog::warn("The port({}) is already in use", port);

            // 获取占用端口的进程 ID
            char cmd[1024];
            std::snprintf(cmd, sizeof(cmd), "netstat -ano | find \"LISTENING\" | find \"%d\"", port);

            FILE *fp = _popen(cmd, "r");
            if (fp == nullptr) {
                spdlog::error("Failed to execute command: {}", cmd);
                closesocket(sockfd);
                WSACleanup();
                return false;
            }

            char buf[1024];
            std::string pid_str;
            while (fgets(buf, 1024, fp) != nullptr) {
                char *p = strrchr(buf, ' ');
                if (p != nullptr) {
                    pid_str = p + 1;
                    break;
                }
            }
            _pclose(fp);
            pid_str.erase(pid_str.find_last_not_of("\n") + 1);

            // 如果获取到了 PID，则杀死该进程
            if (!pid_str.empty()) {
                spdlog::info("Killing the process on port({}): PID={}", port, pid_str);
                ret = std::system(fmt::format("taskkill /F /PID {}", pid_str).c_str());
                if (ret != 0) {
                    spdlog::error("Failed to kill the process: {}", pid_str);
                    closesocket(sockfd);
                    WSACleanup();
                    return false;
                }
                spdlog::info("The process({}) is killed successfully", pid_str);
            } else {
                spdlog::error("Failed to get process ID on port({})", port);
                closesocket(sockfd);
                WSACleanup();
                return false;
            }
        } else {
            spdlog::error("Failed to bind socket: {}", WSAGetLastError());
            closesocket(sockfd);
            WSACleanup();
            return false;
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return true;

#else
    // 创建一个新的套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        spdlog::error("Failed to create socket: {}", std::strerror(errno));
        return false;
    }

    // 绑定到指定端口上
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        if (errno == EADDRINUSE) {
            spdlog::warn("The port({}) is already in use", port);

            // 获取占用端口的进程 ID
            std::string cmd = fmt::format("lsof -i :{} -t", port);
            FILE *fp = popen(cmd.c_str(), "r");
            if (fp == nullptr) {
                spdlog::error("Failed to execute command: {}", cmd);
                close(sockfd);
                return false;
            }

            char buf[1024];
            std::string pid_str;
            while (fgets(buf, 1024, fp) != nullptr) {
                pid_str += buf;
            }
            pclose(fp);
            pid_str.erase(pid_str.find_last_not_of("\n") + 1);

            // 如果获取到了 PID，则杀死该进程
            if (!pid_str.empty()) {
                spdlog::info("Killing the process on port({}): PID={}", port, pid_str);
                int ret = std::system(fmt::format("kill {}", pid_str).c_str());
                if (ret != 0) {
                    spdlog::error("Failed to kill the process: {}", pid_str);
                    close(sockfd);
                    return false;
                }
                spdlog::info("The process({}) is killed successfully", pid_str);
            } else {
                spdlog::error("Failed to get process ID on port({})", port);
                close(sockfd);
                return false;
            }
        } else {
            spdlog::error("Failed to bind socket: {}", std::strerror(errno));
            close(sockfd);
            return false;
        }
    }

    close(sockfd);
    return true;
#endif
}

// 判断是否有相同的程序在运行，并杀死前一个程序
void check_duplicate_process(const std::string& program_name) {
    #ifdef _WIN32 // Windows平台
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        spdlog::error("CreateToolhelp32Snapshot failed: {}", GetLastError());
        exit(EXIT_FAILURE);
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);
    BOOL bRet = Process32First(hSnapshot, &pe);
    while (bRet) {
        std::string name = pe.szExeFile;
        if (name == program_name) {
            spdlog::info("Found duplicate {} process with PID {}", program_name, pe.th32ProcessID);
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
            if (hProcess == NULL) {
                spdlog::error("OpenProcess failed: {}", GetLastError());
                exit(EXIT_FAILURE);
            }
            if (!TerminateProcess(hProcess, 0)) {
                spdlog::error("TerminateProcess failed: {}", GetLastError());
                exit(EXIT_FAILURE);
            }
            CloseHandle(hProcess);
            break;
        }
        bRet = Process32Next(hSnapshot, &pe);
    }
    CloseHandle(hSnapshot);
    #else // Linux和macOS平台
    DIR* dirp = opendir("/proc");
    if (dirp == NULL) {
        spdlog::error("Cannot open /proc directory");
        exit(EXIT_FAILURE);
    }

    std::vector<pid_t> pids;
    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
        if (!isdigit(dp->d_name[0])) {
            continue;
        }
        pid_t pid = atoi(dp->d_name);
        char cmdline_file[256];
        snprintf(cmdline_file, sizeof(cmdline_file), "/proc/%d/cmdline", pid);

        FILE* cmd_file = fopen(cmdline_file, "r");
        if (cmd_file) {
            char cmdline[1024];
            fgets(cmdline, sizeof(cmdline), cmd_file);
            fclose(cmd_file);
            std::string name = cmdline;
            if (name == program_name) {
                pids.push_back(pid);
            }
        }
    }
    closedir(dirp);

    if (pids.size() <= 1) {
        spdlog::info("No duplicate {} process found", program_name);
        return;
    }

    for (auto pid : pids) {
        spdlog::info("Found duplicate {} process with PID {}", program_name, pid);
        if (kill(pid, SIGTERM) != 0) {
            spdlog::error("kill failed: {}", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    #endif
}

// 判断是否连接网络
void is_network_connected() {
    try {
        #ifdef _WIN32 // Windows平台
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            spdlog::error("WSAStartup failed: {}", result);
            exit(EXIT_FAILURE);
        }

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            spdlog::error("socket failed: {}", WSAGetLastError());
            return false;
        }

        struct sockaddr_in saServer;
        saServer.sin_family = AF_INET;
        saServer.sin_addr.s_addr = inet_addr("8.8.8.8");
        saServer.sin_port = htons(53); // DNS端口

        int nRet = connect(sock, (struct sockaddr*)&saServer, sizeof(saServer));
        closesocket(sock);
        if (nRet == SOCKET_ERROR) {
            spdlog::error("connect failed: {}", WSAGetLastError());
            return false;
        }
        return true;
        #else // Linux和macOS平台
        addrinfo hint, *res;
        std::memset(&hint, 0, sizeof(struct addrinfo));
        hint.ai_family = AF_UNSPEC;
        hint.ai_socktype = SOCK_STREAM;
        hint.ai_flags = AI_CANONNAME;

        if (getaddrinfo("www.baidu.com", "http", &hint, &res) != 0) {
            spdlog::error("getaddrinfo failed: {}", strerror(errno));
            return;
        }

        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock == -1) {
            spdlog::error("socket failed: {}", strerror(errno));
            freeaddrinfo(res);
            return;
        }

        if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
            spdlog::error("connect failed: {}", strerror(errno));
            close(sock);
            freeaddrinfo(res);
            return;
        }

        close(sock);
        freeaddrinfo(res);

        spdlog::info("Network checked , connected!");
        #endif
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }
}

void LoadUrl() {
    CROW_ROUTE(app, "/")
    ([]{
        return crow::mustache::load("index.html").render();
    });

    CROW_ROUTE(app, "/client")
    ([]{
        return crow::mustache::load("client.html").render();
    });
}

void TestAll() {

    // 测试模组管理器

    spdlog::debug("The following output is just for debugging");

    spdlog::debug("Test ModuleLoader and some important functions :");

    spdlog::debug("Test Module Loading Functions : {}",m_ModuleLoader.LoadModule("modules/test/libmylib.so","mylib"));
    
    m_ModuleLoader.LoadAndRunFunction<void>("mylib","my_func","test",false);

    // 严重bug
    //spdlog::debug("Test Get all of the functions in modules {}",m_ModuleLoader.getFunctionList("mylib").dump());

    spdlog::debug("Test HasModule Functions by looking for a none modules: {}",m_ModuleLoader.HasModule("fuckyou"));

    //测试任务管理器

    std::shared_ptr<OpenAPT::ConditionalTask> conditionalTask(new OpenAPT::ConditionalTask(
        []() { spdlog::info("conditional task executed!"); },
        {{"threshold", 10}},
        [](const json& params) -> bool { 
            spdlog::info("Conditon function was called!");
            return params["threshold"].get<int>() > 5; 

            }
    ));

    m_TaskManager.addTask(conditionalTask);

    m_TaskManager.addTask(m_TaskManager.m_TaskGenerator.generateSimpleTask("simpleTask","Just a test",{},"","Print"));

    m_TaskManager.addTask(m_TaskManager.m_TaskGenerator.generateSimpleTask("simpleTaska","Just a test",{},"mylib","my_func"));

    m_TaskManager.addTask(m_TaskManager.m_TaskGenerator.generateConditionalTask("conditionalTask", "A test conditional task", {{"status", 2}}));

    m_TaskManager.executeAllTasks();

    // 测试设备调度

    m_DeviceManager.addDevice(OpenAPT::DeviceType::Camera, "Camera1");

    auto cameraList = m_DeviceManager.getDeviceList(OpenAPT::DeviceType::Camera);
    std::cout << "相机列表: ";
    for (auto& name : cameraList) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    auto telescopeList = m_DeviceManager.getDeviceList(OpenAPT::DeviceType::Telescope);
    std::cout << "望远镜列表: ";
    for (auto& name : telescopeList) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    // 测试配置管理器
    m_ConfigManager.setValue("key1", "value1");
    m_ConfigManager.setValue("key2/inner_key", 3.1415926);
    spdlog::info("Get value of key2/inner_key: {}", m_ConfigManager.getValue("key2/inner_key").dump());
    m_ConfigManager.printAllValues();

}


void quit() {
    ::exit(1);
}

int main(int argc, char* argv[]) {

    try {
        registerInterruptHandler();

        parse_args(argc, argv);

        check_duplicate_process(argv[0]);

        LoadUrl();

        if (DEBUG) {
            spdlog::set_level(spdlog::level::debug);
            app.loglevel(crow::LogLevel::DEBUG);
            TestAll();
        } else {
            spdlog::set_level(spdlog::level::info);
            app.loglevel(crow::LogLevel::ERROR);
        }

        bool ret = CheckAndKillProgramOnPort(8000);
        if (!ret) {
            quit();
        }

        CROW_WEBSOCKET_ROUTE(app, "/app")
        .onopen([&](crow::websocket::connection& conn) {
            spdlog::info("WebSocket connection opened.");
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
            spdlog::warn("WebSocket connection closed. Reason: {}", reason);
        })
        .onmessage([&](crow::websocket::connection& /*conn*/, const std::string& data, bool is_binary) {
            try {
                auto j = json::parse(data); // 解析 JSON
                std::string event = j["event"];
                std::string message = j["message"];
                std::string remote_event = j["remote_event"]; // 获取远程事件类型

                if (event == "start_coroutine") {
                    spdlog::info("Starting coroutine...");
                    //co_await process_event_in_coroutine(message, remote_event);
                } else if (event == "start_thread") {
                    spdlog::info("Starting thread...");
                    //std::thread t(process_event_in_thread, message);
                    //t.detach();
                } else if (event == "start_process") {
                    spdlog::info("Starting process...");
                    //std::system("python my_script.py");
                } else {
                    spdlog::error("Invalid event type: {}", event);
                }
            } catch (const json::exception &e) {
                spdlog::error("Failed to parse JSON: {}", e.what());
            }
        });

        app.port(8000).multithreaded().run(); // 启动 Web 服务器

    } catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
			// 保存崩溃日志到文件中
			OpenAPT::CrashReport::saveCrashLog(e.what());
			std::exit(EXIT_FAILURE);
	}
    

    return 0;
}
