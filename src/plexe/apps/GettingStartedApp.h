
#pragma once

#include "plexe/apps/SimplePlatooningApp.h"
#include "plexe/scenarios/BaseScenario.h"
#include "plexe/messages/AbandonPlatoon_m.h"
#include "plexe/messages/NewFormation_m.h"
#include "plexe/messages/PlexeInterfaceControlInfo_m.h"

namespace plexe {

class GettingStartedApp : public SimplePlatooningApp {

public:
    GettingStartedApp()
    {
    }

    void sendAbandonMessage();
    virtual void sendUnicast(cPacket* msg, int destination);

protected:
    /** override from SimplePlatooningApp */
    virtual void initialize(int stage) override;
    virtual void handleLowerMsg(cMessage* msg) override;

    BaseScenario* scenario;

private:
    AbandonPlatoon* createAbandonMessage();
    NewFormation* createNewFormationMessage(const std::vector<int>& newPlatoonFormation);
    void handleAbandonPlatoon(const AbandonPlatoon* msg);
    void handleNewFormation(const NewFormation* msg);
    void sendNewFormationToFollowers(const std::vector<int>& newPlatoonFormation);
};

} // namespace plexe

