/*
 * indiTelescope.hpp
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

Date: 2023-4-10

Description: INDI Telescope

**************************************************/

#pragma once

#include "api/indiclient.hpp"
#include "device/basic_device.hpp"

#include <libindi/basedevice.h>
#include <libindi/indiproperty.h>

#include <string>

#include <spdlog/spdlog.h>

namespace OpenAPT
{
    class INDITelescope : public Telescope, public OpenAptIndiClient
    {
        // INDI Parameters
    private:
        ISwitchVectorProperty *connection_prop;
        INumberVectorProperty *telescopeinfo_prop;
        ITextVectorProperty *telescope_port;
        ISwitchVectorProperty *rate_prop;
        INDI::BaseDevice *telescope_device;

        bool is_ready;
        bool has_blob;

        std::string indi_telescope_port = "";
        std::string indi_telescope_rate = "";

        std::string indi_telescope_cmd;
        std::string indi_telescope_exec = "";
        std::string indi_telescope_version = "";
        std::string indi_telescope_interface = "";

    public:

        INDITelescope(const std::string &name);
        ~INDITelescope();

        bool connect(std::string name) override;
        bool disconnect() override;
        bool reconnect() override;
        bool scanForAvailableDevices() override;

        bool SlewTo(const std::string & ra,const std::string &dec,const bool j2000 = false)  override;
        bool Abort() override;

        bool StartTracking(const std::string &model,const std::string &speed) override;
        bool StopTracking() override;
        bool setTrackingMode(const std::string &mode) override;
        bool setTrackingSpeed(const std::string &speed) override;

        bool Home() override;
        bool isAtHome() override;
        bool setHomePosition() override;

        bool Park() override;
        bool Unpark() override;
        bool isAtPark() override;
        bool setParkPosition() override;

        // 获取简单任务
        std::shared_ptr<OpenAPT::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) override;
        // 获取条件任务
        std::shared_ptr<OpenAPT::ConditionalTask> getCondtionalTask(const std::string &task_name, const nlohmann::json &params) override;
        // 获取循环任务
        std::shared_ptr<OpenAPT::LoopTask> getLoopTask(const std::string &task_name, const nlohmann::json &params) override;

    protected:
        void ClearStatus();

    protected:
        void newDevice(INDI::BaseDevice *dp) override;
        void removeDevice(INDI::BaseDevice *dp) override;
        void newProperty(INDI::Property *property) override;
        void removeProperty(INDI::Property *property) override {}
        void newBLOB(IBLOB *bp) override;
        void newSwitch(ISwitchVectorProperty *svp) override;
        void newNumber(INumberVectorProperty *nvp) override;
        void newMessage(INDI::BaseDevice *dp, int messageID) override;
        void newText(ITextVectorProperty *tvp) override;
        void newLight(ILightVectorProperty *lvp) override {}
        void IndiServerConnected() override;
        void IndiServerDisconnected(int exit_code) override;
    };

}