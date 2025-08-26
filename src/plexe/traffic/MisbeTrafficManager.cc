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

#include "MisbeTrafficManager.h"

namespace plexe {

Define_Module(MisbeTrafficManager);

void MisbeTrafficManager::initialize(int stage)
{
    HeterogeneousControllerTrafficManager::initialize(stage);

    if (stage == 0) {
        trafficNoiseAround = par("trafficNoiseAround").boolValue();
        numNoisyVehicles = par("numNoisyVehicles").intValue();
        noisyvehVType = par("noisyvehVType").stringValue();
        maxPlatoLength = par("maxPlatoLength").doubleValue();
        currentPos = par("currentPos").doubleValue();
        lane = par("lane").intValue();

        insertNoisyTrafficMessage = new cMessage("insertNoisyTrafficMessage");
        scheduleAt(platoonInsertTime + 1.0, insertNoisyTrafficMessage);
    }
}

MisbeTrafficManager::~MisbeTrafficManager()
{
    cancelAndDelete(insertNoisyTrafficMessage);
    insertNoisyTrafficMessage = nullptr;
}

void MisbeTrafficManager::handleSelfMsg(cMessage* msg)
{
    HeterogeneousControllerTrafficManager::handleSelfMsg(msg);
    if (msg == insertNoisyTrafficMessage)
        if (trafficNoiseAround)
            insertNoisyTraffic();
}

void MisbeTrafficManager::insertNoisyCar(int lane, double speed, double desiredSpeed, double position)
{
    Vehicle traci_info = {
        .id = findVehicleTypeIndex(noisyvehVType),
        .lane = lane,
        .position = static_cast<float>(position),
    };
    traci_info.speed = (float) speed;
    //traci_info.ccDesiredSpeed = desiredSpeed;
    this->addVehicleToQueue(0, traci_info);
    // populate also Plexe managers
    VehicleInfo vehicle_info = {
        .controller = ACC, // initially vehs are all DRIVER
        .distance = 5, // if ever used by a PATH veh...
        .headway = 1.2,
        .id = this->currentVehicleId,
        .platoonId = this->currentVehicleId++,
        .position = 0,
    };
    PlatoonInfo platoon_info{
        .speed = traci_info.speed,
        .lane = traci_info.lane};

    this->positions.addVehicleToPlatoon(vehicle_info.id, vehicle_info);
    this->positions.setPlatoonInformation(vehicle_info.platoonId, platoon_info);

}

/*
 * This is the trafficManger only for the Misbehavior study
 *
 * Now...
 * We assume that platoon will be inserted at time 1s in lane0
 * We will place after 1s <numNoisyVehicles> into lanes 1 and 2
 *
 */
void MisbeTrafficManager::insertNoisyTraffic()
{
    double spacingPerTwoVehicles = maxPlatoLength / (numNoisyVehicles * 2);

    for (int i = 0; i < numNoisyVehicles; i ++) {
        lane = i%2 + 1; // either lane 1 or 2
        insertNoisyCar(lane, platoonInsertSpeed, platoonInsertSpeed, currentPos-par("goBackBy").doubleValue());
        if (lane==2)
            currentPos -= spacingPerTwoVehicles;
    }
}

} // namespace plexe

