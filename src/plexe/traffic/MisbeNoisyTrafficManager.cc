#include "MisbeNoisyTrafficManager.h"
#include <cmath>
#include "plexe/utilities/utilities.h"

namespace plexe {

Define_Module(MisbeNoisyTrafficManager);

void MisbeNoisyTrafficManager::initialize(int stage)
{
    MisbeTrafficManager::initialize(stage);

    if (stage == 0) {
        trafficNoiseAround = par("trafficNoiseAround").boolValue();
        numNoisyVehicles = par("numNoisyVehicles").intValue();
        noisyvehVType = par("noisyvehVType").stringValue();
        currentPos = par("currentPos").doubleValue();
        lane = par("lane").intValue();
        // we should start injecting vehicles from next id after platooningVehs
        currentVehicleId = par("nCars").intValue();

        if (trafficNoiseAround) {
            insertNoisyTrafficMessage = new cMessage("insertNoisyTrafficMessage");
            scheduleAt(platoonInsertTime + 1.0, insertNoisyTrafficMessage);
        }
    }
}

MisbeNoisyTrafficManager::~MisbeNoisyTrafficManager()
{
    cancelAndDelete(insertNoisyTrafficMessage);
    insertNoisyTrafficMessage = nullptr;
}

void MisbeNoisyTrafficManager::handleSelfMsg(cMessage* msg)
{
    MisbeTrafficManager::handleSelfMsg(msg);
    if (msg == insertNoisyTrafficMessage)
        if (trafficNoiseAround)
            insertNoisyTraffic();
}

void MisbeNoisyTrafficManager::insertNoisyCar(int lane, double speed, double position)
{
    Vehicle traci_info = {
        .id = findVehicleTypeIndex(noisyvehVType),
        .lane = lane,
        .position = static_cast<float>(position),
    };
    traci_info.speed = (float) speed;
    this->addVehicleToQueue(0, traci_info);
    // populate also Plexe managers
    VehicleInfo vehicle_info = {
        .controller = ACC, // initially vehs are all DRIVER
        .distance = 5, // if ever used by a PATH veh...
        .headway = 1.2,
        .id = currentVehicleId,
        .platoonId = currentVehicleId++,
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
void MisbeNoisyTrafficManager::insertNoisyTraffic()
{
    // insert over 2 lanes, first half on lane...
    for (int i = 0; i < numNoisyVehicles/2; i ++) {
        insertNoisyCar(lane, platoonInsertSpeed / 3.6, currentPos);
        currentPos -= par("goBackBy").doubleValueInUnit("m");
    }
    // moving to next lane...
    lane++;
    currentPos = par("currentPos").doubleValue();
    for (int i = numNoisyVehicles/2; i < numNoisyVehicles; i ++) {
        insertNoisyCar(lane, platoonInsertSpeed / 3.6, currentPos);
        currentPos -= par("goBackBy").doubleValueInUnit("m");
    }
}

} // namespace plexe
