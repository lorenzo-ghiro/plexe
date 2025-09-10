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
#ifndef MISBEHAVIORSCENARIO_H_
#define MISBEHAVIORSCENARIO_H_

#include "plexe/scenarios/SinusoidalScenario.h"
#include <vector>
#include <string>

namespace plexe {

class DefenseApp;

class MisbehaviorScenario : public SinusoidalScenario {

public:
    virtual void initialize(int stage) override;

    MisbehaviorScenario()
    {
        misbehaviorMsg = nullptr;
        selectRandomIndexMsg = nullptr;
        platoonSize = 4;
        idMisbehavior = 0;
        timeMisbehavior = 0;
        misbehavior = "constPos";
        defenseAppl = 0;
    }

    virtual ~MisbehaviorScenario();

    int getIdMisbehavior() const {
        return idMisbehavior;
    }

    void setIdMisbehavior(int idMisbehavior) {
        this->idMisbehavior = idMisbehavior;
    }

protected:
    // vector of vehicle's that will have
    // a misbehaviour about the position
    //std::vector<int> ids;

    // vector of times related to the ids
    // at which there will be a misbehaviour
    //std::vector<double> times;

    // type of misbehaviour 1=constPos,
    // 2=randomPos, 3=RandomOffset
    //std::vector<const char*> types;

    // message used to tell to the vehicle with id i to send the same position
    cMessage* misbehaviorMsg;

    // in case of disruptive select a random index
    // for every message to be sent
    cMessage* selectRandomIndexMsg;

    // platoon's size
    int platoonSize;

    // misbehavior vehicle's id
    int idMisbehavior;

    // misbehavior starting time
    double timeMisbehavior;

    // mishavior's type
    std::string misbehavior;


    virtual void handleSelfMsg(cMessage* msg) override;

    DefenseApp* defenseAppl;

};

} // namespace plexe

#endif
