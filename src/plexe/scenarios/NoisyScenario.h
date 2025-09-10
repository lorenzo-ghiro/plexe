#ifndef NOISYSCENARIO_H_
#define NOISYSCENARIO_H_

#include "plexe/scenarios/MisbehaviorScenario.h"

namespace plexe {

class NoisyScenario : public MisbehaviorScenario {

public:
    virtual void initialize(int stage) override;

    virtual ~NoisyScenario();

    int numInitStages() const override { return 4; }

protected:
    int numNoisyVehicles;
    int idNoisyAttacker;

    virtual void handleSelfMsg(cMessage* msg) override;
};

} // namespace plexe

#endif
