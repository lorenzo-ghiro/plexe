//
// Copyright (C) 2018-2021 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "plexe/scenarios/GettingStartedScenario.h"

using namespace veins;

namespace plexe {

Define_Module(GettingStartedScenario);

void GettingStartedScenario::initialize(int stage)
{

    BaseScenario::initialize(stage);

    if (stage == 0)
        // get pointer to application
        appl = FindModule<GettingStartedApp*>::findSubModule(getParentModule());

    if (stage == 2) {
        // average speed
        leaderSpeed = par("leaderSpeed").doubleValue() / 3.6;

        if (positionHelper->isLeader()) {
            // set base cruising speed
            plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed);
        }
        else {
            // let the follower set a higher desired speed to stay connected
            // to the leader when it is accelerating
            plexeTraciVehicle->setCruiseControlDesiredSpeed(leaderSpeed + 10);

            // if we are the last vehicle, then schedule breaking at time 10sec
            std::vector<int> pFormation = positionHelper->getPlatoonFormation();
            // if we are the last vehicle...
            if (positionHelper->getId() == pFormation[pFormation.size() - 1]) {
                // prepare self messages for scheduled operations
                startBreaking = new cMessage("Start Breaking now!");
                checkDistance = new cMessage("Check Distance now!");

                // ...then schedule Break operation
                scheduleAt(10, startBreaking);
            }
        }
    }
}


void GettingStartedScenario::handleMessage(cMessage* msg)
{
    if (msg == startBreaking) {
        // Increase CACC Constant Spacing (set it to 15m)
        plexeTraciVehicle->setCACCConstantSpacing(15.0);
        traciVehicle->setColor(TraCIColor(100, 100, 100, 255));
        // then start checking when we reach that 15m distance
        scheduleAt(simTime() + 0.1, checkDistance);

    }
    else if (msg == checkDistance) {
        // Checking current distance with radar
        double distance = nan("nan"), relSpeed = nan("nan");
        plexeTraciVehicle->getRadarMeasurements(distance, relSpeed);
        std::cout << "LEAVING VEHICLE now at: " << distance << " meters" << endl;

        if (distance > 14.9) {
            // We are almost at correct distance! Turn to ACC (i.e., abandon platoon...)
            plexeTraciVehicle->setActiveController(ACC);
            plexeTraciVehicle->setACCHeadwayTime(1.2);
            traciVehicle->setColor(TraCIColor(200, 200, 200, 255));

            // send abandon Platoon message to leader
            appl->sendAbandonMessage();
        }
        else {
            scheduleAt(simTime() + 0.1, checkDistance);
        }
    }
}

GettingStartedScenario::~GettingStartedScenario()
{
    cancelAndDelete(startBreaking);
    cancelAndDelete(checkDistance);
}

} // namespace plexe
