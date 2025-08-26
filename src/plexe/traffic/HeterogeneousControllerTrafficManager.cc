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

#include "HeterogeneousControllerTrafficManager.h"
#include <cmath>
#include "plexe/utilities/utilities.h"

namespace plexe {

Define_Module(HeterogeneousControllerTrafficManager);

void HeterogeneousControllerTrafficManager::initialize(int stage)
{

    TraCIBaseTrafficManager::initialize(stage);

    if (stage == 0) {

        nCars = par("nCars");
        platoonSize = par("platoonSize");
        nLanes = par("nLanes");
        platoonInsertTime = SimTime(par("platoonInsertTime").doubleValue());
        platoonInsertSpeed = par("platoonInsertSpeed").doubleValueInUnit("mps");
        platoonLeaderHeadway = par("platoonLeaderHeadway").doubleValue();
        platoonAdditionalDistance = par("platoonAdditionalDistance").doubleValue();
        platooningVType = par("platooningVType").stdstringValue();
        createSubPlatoons = par("createSubPlatoons").boolValue();
        useRandomControllerAssignment = par("useRandomControllerAssignment").boolValue();
        timeMisbehavior = par("timeMisbehavior").doubleValue();
        parsePlatoonDistancesAndHeadways();

        insertPlatoonMessage = new cMessage("");
        scheduleAt(platoonInsertTime, insertPlatoonMessage);
    }
}

void HeterogeneousControllerTrafficManager::parseController()
{
    strController = par("controller").stringValue();
    std::cout << "strController = " << strController << " len = " << strController.size() << "\n";

    if (strController.size() == 0 && !useRandomControllerAssignment)
        throw cRuntimeError("Invalid controller selected");

    else if (strController.size() == 1 && !useRandomControllerAssignment) {
        // Case where we must use only one controller, e.g., "PLOEG"
        // set the controller for the leader
        controllers.push_back(ACC);
        // set the same (unique) controller for all the others
        for (int i = 1; i < platoonSize; i++) {
            controller = charToController(strController[0]);
            controllers.push_back(controller);
        }
    }
    else if (strController.size() > 1 && !useRandomControllerAssignment) {
        // Multiple controllers listed in the "controller" string
        // assign the controllers as per the controller parameter
        for (int i = 0; i < platoonSize; i++)
            controllers.push_back(charToController(strController[i % strController.size()]));
    }
    else if (useRandomControllerAssignment) {
        // assign the controllers randomly
        controllers.push_back(ACC); // first car always ACC
        enum ACTIVE_CONTROLLER randomControllers[] = {CACC, PLOEG};
        for (int i = 1; i < platoonSize; i++)
            controllers.push_back(randomControllers[intuniform(0, 2)]);
    }
    else
        throw cRuntimeError("Invalid controller selected");

    std::cout << "Formation: ";
    for (auto ctrl : controllers)
        std::cout << controllerToString(ctrl) << " ";
    std::cout << "\n";
}

void HeterogeneousControllerTrafficManager::parsePlatoonDistancesAndHeadways()
{
    strPlatoonDistances = par("platoonDistances").stdstringValue();
    strPlatoonHeadways = par("platoonHeadways").stdstringValue();
    std::vector<std::string> distancesVector = cStringTokenizer(strPlatoonDistances.c_str()).asVector();
    std::vector<std::string> headwaysVector = cStringTokenizer(strPlatoonHeadways.c_str()).asVector();

    if (distancesVector.size() != headwaysVector.size())
        throw cRuntimeError("The lengths of the 'platoonDistances' and 'platoonHeadways' parameters do not match");

    for (int i = 0; i < distancesVector.size(); i++) {
        std::vector<std::string> distElement = cStringTokenizer(distancesVector[i].c_str(), "=").asVector();
        if (distElement.size() != 2)
            throw cRuntimeError("Invalid parameter %s. Expecting <CONTROLLER>=<VALUE>", distancesVector[i].c_str());
        platoonDistances[strToController(distElement[0].c_str())] = std::stod(distElement[1]);

        std::vector<std::string> headwayElement = cStringTokenizer(headwaysVector[i].c_str(), "=").asVector();
        if (headwayElement.size() != 2)
            throw cRuntimeError("Invalid parameter %s. Expecting <CONTROLLER>=<VALUE>", headwaysVector[i].c_str());
        platoonHeadways[strToController(headwayElement[0].c_str())] = std::stod(headwayElement[1]);
    }
}

void HeterogeneousControllerTrafficManager::scenarioLoaded()
{
    automated.id = findVehicleTypeIndex(platooningVType);
    automated.lane = -1;
    automated.position = 0;
    automated.speed = platoonInsertSpeed;
}

void HeterogeneousControllerTrafficManager::handleSelfMsg(cMessage* msg)
{
    TraCIBaseTrafficManager::handleSelfMsg(msg);
    if (msg == insertPlatoonMessage)
        insertPlatoons();
}

double HeterogeneousControllerTrafficManager::computePlatoonLength()
{
    // start with the length of the leader
    double length = 4;
    for (int i = 1; i < platoonSize; i++) {
        length += 4 + platoonInsertSpeed * platoonHeadways[controllers[i]] + platoonDistances[controllers[i]];
        // Mod 1 to modify start position and speed for transient analysis
        // length += 4 + 2;
    }
    return length;
}

void HeterogeneousControllerTrafficManager::insertPlatoons()
{
    // total number of platoons per lane
    int nPlatoons = nCars / platoonSize / nLanes;
    // length of 1 platoon
    double platoonLength = computePlatoonLength();
    // inter-platoon distance
    // Mod 2 to modify start position and speed for transient analysis
    // double platoonDistance = platoonAdditionalDistance;
    double platoonDistance = platoonInsertSpeed * platoonLeaderHeadway + platoonAdditionalDistance;
    // total length for one lane
    double totalLength = nPlatoons * platoonLength + (nPlatoons - 1) * platoonDistance;

    // for each lane, we create an offset to have misaligned platoons
    double* laneOffset = new double[nLanes];
    for (int l = 0; l < nLanes; l++) laneOffset[l] = uniform(0, 20);

    int currentVehiclePosition = 0;
    currentVehicleId = 0;
    int currentLeaderPosition = 0;
    int currentPlatoonId = 0;
    enum ACTIVE_CONTROLLER front_cntrl;
    for (int l = 0; l < nLanes; l++) {
        double currentRoadPosition = totalLength;
        for (int i = 0; i < nCars / nLanes; i++) {
            VehicleInfo vehicleInfo;
            vehicleInfo.controller = controllers[currentVehiclePosition];
            if (currentVehiclePosition > 0)
                front_cntrl = controllers[currentVehiclePosition - 1];
            else
                front_cntrl = DRIVER; // dummy value

            vehicleInfo.id = currentVehicleId;
            vehicleInfo.position = i;
            vehicleInfo.platoonId = currentPlatoonId;
            vehicleInfo.distance = platoonDistances[vehicleInfo.controller];
            vehicleInfo.headway = platoonHeadways[vehicleInfo.controller];
            if (createSubPlatoons) {
                // if the controller for the current vehicle is different from the one in front
                // we create a subplatoon with a new leader
                if (vehicleInfo.controller != front_cntrl)
                    currentLeaderPosition = i - 1;
            }
            vehicleInfo.leaderPosition = currentLeaderPosition;

            automated.position = currentRoadPosition + laneOffset[l];
            automated.lane = l;
            automated.vehicleId = currentVehicleId;
            automated.routeid = 0;
            addVehicleToQueue(automated.routeid, automated);
            positions.addVehicleToPlatoon(currentVehicleId, vehicleInfo);
            currentVehicleId++;
            if (i == 0) {
                PlatoonInfo info;
                info.speed = automated.speed;
                info.lane = automated.lane;
                positions.setPlatoonInformation(vehicleInfo.platoonId, info);
            }
            currentVehiclePosition++;
            if (currentVehiclePosition == platoonSize) {
                currentVehiclePosition = 0;
                // add inter platoon gap
                currentRoadPosition -= (platoonDistance + 4);
            }
            else {
                // add intra platoon gap
                enum ACTIVE_CONTROLLER cntrl = controllers[currentVehiclePosition];
                // Mod 3 to modify start position and speed for transient analysis
                // currentRoadPosition -= 4 + 2;
                currentRoadPosition -= (4 + platoonDistances[cntrl] + platoonInsertSpeed * platoonHeadways[cntrl]);
            }
        }
        currentPlatoonId++;
    }

    delete[] laneOffset;
}

HeterogeneousControllerTrafficManager::~HeterogeneousControllerTrafficManager()
{
    cancelAndDelete(insertPlatoonMessage);
    insertPlatoonMessage = nullptr;
}

void HeterogeneousControllerTrafficManager::setReactionTime(double react)
{
	reactionTime = react;
}

void HeterogeneousControllerTrafficManager::finish()
{
	recordScalar("reactionTime", reactionTime);
	recordScalar("timeMisbehavior", timeMisbehavior);
}


} // namespace plexe
