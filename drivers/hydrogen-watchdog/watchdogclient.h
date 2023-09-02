/*******************************************************************************
  Copyright(c) 2015 Jasem Mutlaq. All rights reserved.

  HYDROGEN Watchdog Client.

  The clients communicates with HYDROGEN server to put devices in a safe state for shutdown

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/

#pragma once

#include "baseclient.h"
#include "basedevice.h"

#include <cstring>

class WatchDogClient : public HYDROGEN::BaseClient
{
    public:
        WatchDogClient();
        ~WatchDogClient();

        bool isBusy()
        {
            return isRunning;
        }
        bool isConnected()
        {
            return isReady;
        }

        void setDome(const std::string &value);
        void setMount(const std::string &value);

        bool parkDome();
        bool parkMount();
        IPState getDomeParkState();
        IPState getMountParkState();

    protected:
        virtual void newDevice(HYDROGEN::BaseDevice dp) override;
        virtual void newProperty(HYDROGEN::Property property) override;

    private:
        std::string dome, mount;
        bool isReady, isRunning, domeOnline, mountOnline;

        HYDROGEN::PropertyViewSwitch *mountParkSP, *domeParkSP;
};
