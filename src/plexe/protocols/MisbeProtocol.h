#ifndef SRC_PLEXE_PROTOCOLS_MISBEPROTOCOL_H_
#define SRC_PLEXE_PROTOCOLS_MISBEPROTOCOL_H_

#include "veins/modules/messages/BaseFrame1609_4_m.h"

#include "plexe/messages/PlatooningBeacon_m.h"
#include "plexe/protocols/SimplePlatooningBeaconing.h"

namespace plexe {

using veins::BaseFrame1609_4;

class MisbeProtocol : public SimplePlatooningBeaconing {

protected:
    // include the misbehaviour warning in beacon
    bool warning = false;

    // define when to create and attack to handle the message
    bool onAttack;
    // define the type of attack for the current vehicle
    const char* attackType;

    // misbehavior positions to be transmitted
    double posx;
    double posy;

    // misbehavior offset defined
    double offset;

    // misbehavior speeds to be transmitted
    double spdx;
    double spdy;

    // acceleration value for the eventual stop attack
    double acl;

    // data saved to be replayed along with the index selected
    struct VEHICLE_DATA replayData;
    int replayIndex;

    // data saved to be replayed along with a random index
    struct VEHICLE_DATA disruptiveData;

    virtual void initialize(int stage) override;

    virtual void handleSelfMsg(cMessage* msg) override;

    /**
     * Sending a platooning message with all information about the car but the pos misbehaviour
     */
    void sendMisbehaviorMessage(int destinationAddress, enum PlexeRadioInterfaces interfaces = PlexeRadioInterfaces::ALL);

    /**
     * Sending a platooning message of replay kind, given the replayBeacon
     */
    void sendReplayMessage(int destinationAddress, enum PlexeRadioInterfaces interfaces = PlexeRadioInterfaces::ALL);

    /**
     * Sending a platooning message of replay kind, from random vehicles
     */
    void sendDisruptiveMessage(int destinationAddress, enum PlexeRadioInterfaces interfaces = PlexeRadioInterfaces::ALL);

    virtual std::unique_ptr<BaseFrame1609_4> createMisbehaviorBeacon(int destinationAddress);

    virtual std::unique_ptr<BaseFrame1609_4> createReplayBeacon(int destinationAddress, VEHICLE_DATA attackData);

    virtual std::unique_ptr<BaseFrame1609_4> createBeacon(int destinationAddress) override;

public:
    MisbeProtocol() {
        replayIndex = -1;
        onAttack = false;
        posx = 0;
        posy = 0;
        offset = 0;
        spdx = 0;
        spdy = 0;
        acl = 0;
    }

    // set the warning in sent beacons
    void setWarning(bool misbehaviour);
    // set the onAttack variable
    void activeAttack(const char* type);
    // set the message to be replayed in case of replay attack
    void setReplayMessage(const PlatooningBeacon* pb);
    // set the index to replay in case of data replay attack
    void setReplayIndex(int index);
    // set the message to be replayed in case of disruptive attack
    void setDisruptiveMessage(const PlatooningBeacon* pb);
};

} // namespace plexe

#endif /* SRC_PLEXE_PROTOCOLS_MISBEPROTOCOL_H_ */
