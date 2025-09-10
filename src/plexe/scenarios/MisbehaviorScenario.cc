//
// Copyright (C) 2018-2023 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "plexe/scenarios/MisbehaviorScenario.h"
#include "plexe/apps/DefenseApp.h"

#include <iostream>
#include <string>
#include <cstring>
#include <random>

#undef NDEBUG
using namespace veins;

namespace plexe {

Define_Module(MisbehaviorScenario);

void MisbehaviorScenario::initialize(int stage)
{
	SinusoidalScenario::initialize(stage);

	// eventually read csv file for multiple attacks
	if (stage == 2){
	    defenseAppl = FindModule<DefenseApp*>::findSubModule(getParentModule());

	    platoonSize = par("platoonSize").intValue();
	    idMisbehavior = par("idMisbehavior").intValue();
	    misbehavior = par("misbehavior").stringValue();

        int myPos = positionHelper->getPosition();
        cModule* traffic = this->getModuleByPath("^.^.traffic");
        timeMisbehavior = traffic->par("timeMisbehavior").doubleValue();
        // generate a random in the platoon excluding
        // the actual vehicle and the one that has to
        // receive
        if(myPos == idMisbehavior and strcmp(misbehavior.c_str(), "dataReplay") == 0){
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(-1, platoonSize-1);

            int randomNum;

            // not my messages or the destination vehicle
            do {
                randomNum = dis(gen);
            } while (randomNum == myPos);
            defenseAppl->setReplayIndex(randomNum);
        }

        if(myPos == idMisbehavior and strcmp(misbehavior.c_str(), "disruptive") == 0){
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(-1, platoonSize-1);

            int randomNum;

            // not my messages or the destination vehicle
            do {
                randomNum = dis(gen);
            } while (randomNum == myPos);
            defenseAppl->setReplayIndex(randomNum);
            selectRandomIndexMsg = new cMessage(misbehavior.c_str());
            scheduleAt(SimTime(timeMisbehavior), selectRandomIndexMsg);
        }

        if (myPos == idMisbehavior) {
            misbehaviorMsg = new cMessage(misbehavior.c_str());
            scheduleAt(SimTime(timeMisbehavior), misbehaviorMsg);
        }
    }
}

MisbehaviorScenario::~MisbehaviorScenario()
{
    cancelAndDelete(misbehaviorMsg);
    misbehaviorMsg = nullptr;
    cancelAndDelete(selectRandomIndexMsg);
    selectRandomIndexMsg = nullptr;
}

void MisbehaviorScenario::handleSelfMsg(cMessage* msg)
{
    SinusoidalScenario::handleSelfMsg(msg);
    if (msg == misbehaviorMsg){
        defenseAppl->activeAttack(msg->getName());
    }

    // if disruptive select a random index every step
    if (msg == selectRandomIndexMsg){
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(-1, platoonSize-1);

        int randomNum;
        // not my messages or the destination vehicle
        do {
            randomNum = dis(gen);
        } while (randomNum == positionHelper->getPosition());
        defenseAppl->setReplayIndex(randomNum);
        scheduleAt(simTime() + SimTime(0.1), selectRandomIndexMsg);
    }
}

} // namespace plexe
