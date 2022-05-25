cplusplus{{
#include "ManeuverMessage_m.h"
}};

packet ManeuverMessage;

// Message sent by a vehicle (as in GettingStarted example)
// to notify the leader that this vehicle left the platoon.
// NB: this message contains also all fields inhereited from ManeuverMessage!
// e.g., vehicleId and platoonId
// NB2: GettingStartedApp shows how to send/receive Maneuver Messages 
packet AbandonPlatoon extends ManeuverMessage {
	string msgContent = "Goodbye I'm leaving";
}