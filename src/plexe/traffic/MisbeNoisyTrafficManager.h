#pragma once

#include "plexe/traffic/MisbeTrafficManager.h"

namespace plexe {

class MisbeNoisyTrafficManager : public MisbeTrafficManager {

public:
    virtual void initialize(int stage) override;

    virtual ~MisbeNoisyTrafficManager();

protected:
    virtual void handleSelfMsg(cMessage* msg) override;

    void insertNoisyTraffic();
    void insertNoisyCar(int lane, double speed, double position);

    // this is used to start traffic around generation
    cMessage* insertNoisyTrafficMessage;

    bool trafficNoiseAround;
    int numNoisyVehicles;
    std::string noisyvehVType;
    double currentPos;
    int lane;
    double goBackBy;
    int currentVehicleId = 0;
};

} // namespace plexe
