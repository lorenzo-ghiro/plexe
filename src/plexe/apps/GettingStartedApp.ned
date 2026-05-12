
package org.car2x.plexe.apps;

import org.car2x.plexe.apps.BaseApp;

simple GettingStartedApp like BaseApp
{
    parameters:
        int headerLength @unit("bit") = default(0 bit);
        @display("i=block/app2");
        @class(plexe::GettingStartedApp);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
}
