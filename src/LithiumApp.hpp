/*
 * lithiumapp.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Lithium App Enter

**************************************************/

#pragma once

#define LITHIUM_APP_MAIN

#include <memory>

#include "atom/server/commander.hpp"
#include "atom/server/message_bus.hpp"
#include "atom/type/message.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

// -------------------------------------------------------------------
// About the LithiumApp
// This is the main class of the Lithium App. All of the functions can be
// executed here. NOTE: No wrapper functions needed, just use the functions
// directly.
//       A json object is used to pass parameters to the functions.
//       And the return value is a json object.
//       Sometimes I think it is unnecessary to use json object to pass
//       parameters. However, It is more convenient to use json object.
// -------------------------------------------------------------------

namespace Atom
{
    namespace Error
    {
        class ErrorStack;
    }

    namespace System
    {
        class ProcessManager;
    }
}
namespace Lithium {
class PyScriptManager;  // FWD

class ComponentManager; // FWD

class ConfigManager;

class TaskPool;

class TaskManager;

class DeviceManager;

class LithiumApp {
public:
    LithiumApp();
    ~LithiumApp();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    static std::shared_ptr<LithiumApp> createShared();

    // -------------------------------------------------------------------
    // Config methods
    // -------------------------------------------------------------------

    json GetConfig(const json &params);
    json SetConfig(const json &params);
    json DeleteConfig(const json &params);
    json SaveConfig(const json &params);

    // -------------------------------------------------------------------
    // Device methods
    // -------------------------------------------------------------------

    json getDeviceList(const json &params);
    json addDevice(const json &params);
    json addDeviceLibrary(const json &params);
    json removeDevice(const json &params);
    json removeDeviceByName(const json &params);
    json removeDeviceLibrary(const json &params);
    /*
            void addDeviceObserver(DeviceType type, const std::string &name);
            bool removeDevice(DeviceType type, const std::string &name);
            bool removeDeviceByName(const std::string &name);
            bool removeDeviceLibrary(const std::string &lib_name);
            std::weak_ptr<Device> getDevice(DeviceType type, const std::string
       &name); size_t findDevice(DeviceType type, const std::string &name);
            std::weak_ptr<Device> findDeviceByName(const std::string &name)
       const; std::weak_ptr<SimpleTask> getTask(DeviceType type, const
       std::string &device_name, const std::string &task_name, const json
       &params); bool getProperty(const std::string &name, const std::string
       &property_name); bool setProperty(const std::string &name, const
       std::string &property_name, const std::string &property_value);
    */

    // -------------------------------------------------------------------
    // Process methods
    // -------------------------------------------------------------------

    json createProcess(const json &params);
    json runScript(const json &params);
    json terminateProcess(const json &params);
    json terminateProcessByName(const json &params);
    json getProcessOutput(const json &params);
    json getRunningProcesses(const json &params);

    // -------------------------------------------------------------------
    // Message methods
    // -------------------------------------------------------------------

    template <typename T>
    void MSSubscribe(const std::string &topic,
                     std::function<void(const T &)> callback,
                     int priority = 0) {
        m_MessageBus.lock()->Subscribe(topic, callback, priority);
    }

    template <typename T>
    void MSUnsubscribe(const std::string &topic,
                       std::function<void(const T &)> callback) {
        m_MessageBus.lock()->Unsubscribe(topic, callback);
    }

    void sendStringMessage(const std::string &topic,
                           const std::string &message) {
        m_MessageBus.lock()->Publish<std::string>(topic, message);
    }

    void sendJsonMessage(const std::string &topic, const json &message) {
        if (message.is_null())
            return;
        if (m_MessageBus.expired())
            return;
        m_MessageBus.lock()->Publish<json>(topic, message);
    }

    ReturnMessage returnMessage(const std::string &message);

public:
    // -------------------------------------------------------------------
    // Lithium Command methods (the main entry point)
    // -------------------------------------------------------------------
    // -------------------------------------------------------------------
    // NOTE: The handler must be registered before the websocket server starts.
    //       Though the handler can be registered after the websocket server
    //       starts, it is not guaranteed that the handler will be called. All
    //       of the servers (both websocket and tcp) will use the follow handler
    //       to handle the command. No private components exposed to theservers.
    // -------------------------------------------------------------------

    void LiRegisterFunc(const std::string &name,
                        std::function<json(const json &)> handler) {
        m_CommandDispatcher->registerHandler(name, handler);
    }

    template <typename T>
    void LiRegisterMemberFunc(const std::string &name,
                              json (T::*memberFunc)(const json &)) {
        if (!m_CommandDispatcher)
            m_CommandDispatcher =
                std::make_unique<CommandDispatcher<json, json>>();
        m_CommandDispatcher->registerMemberHandler(name, this, memberFunc);
    }

    // Max: The async func will be executed in a separate thread, and the return
    // value will be ignored.
    //      So must use MessageBus to send the return value.
    template <typename T>
    void LiRegisterAsyncMemberFunc(const std::string &name,
                                   json (T::*memberFunc)(const json &),
                                   bool async = false) {
        if (!m_CommandDispatcher)
            m_CommandDispatcher =
                std::make_unique<CommandDispatcher<json, json>>();
        m_CommandDispatcher->registerMemberHandler(name + "_async", this,
                                                   memberFunc);
    }

    json DispatchCommand(const std::string &name, const json &params);

    bool hasCommand(const std::string &name);

private:
    std::unique_ptr<CommandDispatcher<json, json>> m_CommandDispatcher;

private:
    std::weak_ptr<TaskPool> m_TaskPool;
    std::weak_ptr<ConfigManager> m_ConfigManager;
    std::weak_ptr<DeviceManager> m_DeviceManager;
    std::weak_ptr<Atom::System::ProcessManager> m_ProcessManager;
    std::weak_ptr<Atom::Server::MessageBus> m_MessageBus;
    std::weak_ptr<Atom::Error::ErrorStack> mStack;
    std::weak_ptr<ComponentManager> m_ComponentManager;
    std::weak_ptr<TaskManager> m_TaskManager;

    std::weak_ptr<PyScriptManager> m_PyScriptManager;
};
extern std::shared_ptr<LithiumApp> MyApp;

void InitLithiumApp(int argc, char **argv);
}  // namespace Lithium
