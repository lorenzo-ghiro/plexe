#include "NoisyScenario.h"
#include "plexe/apps/DefenseApp.h"

#include <random>

namespace plexe {

Define_Module(NoisyScenario);

void NoisyScenario::initialize(int stage)
{

    MisbehaviorScenario::initialize(stage);

    if(stage == 3) {
        numNoisyVehicles = par("numNoisyVehicles").intValue();
        idNoisyAttacker = par("idNoisyAttacker").intValue();
        misbehavior = par("misbehavior").stringValue();

        plexeTraciVehicle->setCruiseControlDesiredSpeed(par("noisyCarSpeed").doubleValueInUnit("mps"));
        plexeTraciVehicle->useRadar(true);

        int myPos = getParentModule()->getIndex();
        cModule* traffic = this->getModuleByPath("^.^.traffic");
        timeMisbehavior = traffic->par("timeMisbehavior").doubleValue();

        if(myPos == idNoisyAttacker and strcmp(misbehavior.c_str(), "dataReplay") == 0){
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(-1, numNoisyVehicles-1);

            int randomNum;

            // not my messages or the destination vehicle
            do {
                randomNum = dis(gen);
            } while (randomNum == myPos);
            defenseAppl->setReplayIndex(randomNum);
        }

        if(myPos == idNoisyAttacker and strcmp(misbehavior.c_str(), "disruptive") == 0){
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(-1, numNoisyVehicles-1);

            int randomNum;

            // not my messages or the destination vehicle
            do {
                randomNum = dis(gen);
            } while (randomNum == myPos);
            defenseAppl->setReplayIndex(randomNum);
            selectRandomIndexMsg = new cMessage(misbehavior.c_str());
            scheduleAt(SimTime(timeMisbehavior), selectRandomIndexMsg);
        }

        if (myPos == idNoisyAttacker) {
            misbehaviorMsg = new cMessage(misbehavior.c_str());
            scheduleAt(SimTime(timeMisbehavior), misbehaviorMsg);
        }
    }
}

NoisyScenario::~NoisyScenario()
{
    cancelAndDelete(misbehaviorMsg);
    misbehaviorMsg = nullptr;
    cancelAndDelete(selectRandomIndexMsg);
    selectRandomIndexMsg = nullptr;
}

void NoisyScenario::handleSelfMsg(cMessage* msg)
{
    SinusoidalScenario::handleSelfMsg(msg);
    if (msg == misbehaviorMsg){
        defenseAppl->activeAttack(msg->getName());
    }

    // if disruptive select a random index every step
    if (msg == selectRandomIndexMsg){
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(-1, numNoisyVehicles-1);

        int randomNum;
        // not my messages or the destination vehicle
        do {
            randomNum = dis(gen);
        } while (randomNum == getParentModule()->getIndex());
        defenseAppl->setReplayIndex(randomNum);
        scheduleAt(simTime() + SimTime(0.1), selectRandomIndexMsg);
    }
}

} // namespace plexe
