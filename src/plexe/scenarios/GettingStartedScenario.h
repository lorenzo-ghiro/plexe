
#pragma once

#include "plexe/scenarios/BaseScenario.h"
#include "plexe/apps/GettingStartedApp.h"

namespace plexe {

class GettingStartedScenario : public BaseScenario {

protected:
    // leader average speed
    double leaderSpeed;
    // application layer, used to stop the simulation
    GettingStartedApp* appl;

private:
    cMessage* startBreaking;
    cMessage* checkDistance;

public:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage* msg) override;
    virtual ~GettingStartedScenario();
    GettingStartedScenario()
        : leaderSpeed(0)
        , appl(nullptr){};
};

} // namespace plexe

