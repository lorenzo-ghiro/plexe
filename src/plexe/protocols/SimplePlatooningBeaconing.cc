//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
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

#include "SimplePlatooningBeaconing.h"

namespace plexe {

Define_Module(SimplePlatooningBeaconing);

void SimplePlatooningBeaconing::initialize(int stage)
{
    BaseProtocol::initialize(stage);

    if (stage == 0) {
        // random start time
        if (beaconingInterval > 0) {
            SimTime beginTime = SimTime(uniform(0.001, beaconingInterval));
            scheduleAt(simTime() + beaconingInterval + beginTime, sendBeacon);
        }
    }
}

void SimplePlatooningBeaconing::handleSelfMsg(cMessage* msg)
{

    BaseProtocol::handleSelfMsg(msg);

    if (msg == sendBeacon) {
        if (!onAttack){
            // not during an attack
            sendPlatooningMessage(-1);
            scheduleAt(simTime() + beaconingInterval, sendBeacon);
        } else if (strcmp(attackType, "dataReplay") == 0) {
            // attack of type replay
            sendReplayMessage(-1);
            scheduleAt(simTime() + beaconingInterval, sendBeacon);
        } else if (strcmp(attackType, "disruptive") == 0) {
            // attack of type disruptive
            sendDisruptiveMessage(-1);
            scheduleAt(simTime() + beaconingInterval, sendBeacon);
        } else {
            // generate random seed for the random attacks
            std::random_device rd;
            std::mt19937 gen(rd());
            if(strcmp(attackType, "randomPos") == 0){
                std::uniform_int_distribution<> distr(lower_bound_random, upper_bound_random);

                posx = distr(gen);
                posy = distr(gen);
            }
            if(strcmp(attackType, "randomOffset") == 0){
                std::uniform_int_distribution<> distr(lower_bound_offset, upper_bound_offset);

                offset = distr(gen);
            }
            if(strcmp(attackType, "randomSpeed") == 0){
                std::uniform_int_distribution<> distr(lower_bound_random_speed, upper_bound_random_speed);

                spdx = distr(gen);
                spdy = distr(gen);
            }
            if(strcmp(attackType, "randomOffsetSpeed") == 0){
                std::uniform_int_distribution<> distr(lower_bound_offset_speed, upper_bound_offset_speed);

                offset = distr(gen);
            }
            sendMisbehaviorMessage(-1);
            scheduleAt(simTime() + beaconingInterval, sendBeacon);
        }
    }
}

SimplePlatooningBeaconing::SimplePlatooningBeaconing()
{
}

SimplePlatooningBeaconing::~SimplePlatooningBeaconing()
{
}

} // namespace plexe
