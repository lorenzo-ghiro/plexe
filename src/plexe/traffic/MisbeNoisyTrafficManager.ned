//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package org.car2x.plexe.traffic;

import org.car2x.plexe.traffic.MisbeTrafficManager;

simple MisbeNoisyTrafficManager extends MisbeTrafficManager {

    parameters:
        bool trafficNoiseAround = default(false);
		int numNoisyVehicles = default(4);
		string noisyvehVType = default("vtypeauto");
		double currentPos = default(2*(3 * (5+1*100/3.6) + 5));
		int lane = default(1);
		volatile double goBackBy @unit("m") = default(uniform(30m, 60m));

        @class(plexe::MisbeNoisyTrafficManager);
}