//
// Copyright (C) 2014-2021 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include "plexe/traffic/HeterogeneousControllerTrafficManager.h"

namespace plexe {

class MisbeTrafficManager : public HeterogeneousControllerTrafficManager {

public:
    virtual void initialize(int stage) override;

    virtual ~MisbeTrafficManager();

protected:
    // this is used to start traffic generation
    cMessage* insertNoisyTrafficMessage;

    virtual void handleSelfMsg(cMessage* msg) override;

    void insertNoisyTraffic();
    void insertNoisyCar(int lane, double speed, double desiredSpeed, double position);

    bool trafficNoiseAround;
    int numNoisyVehicles;
    std::string noisyvehVType;
    double maxPlatoLength;
    double currentPos;
    int lane;
    double goBackBy;
    double noisyCarSpeed;
};

} // namespace plexe
