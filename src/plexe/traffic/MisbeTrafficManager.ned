//
// Copyright (C) 2014-2021 Michele Segata <segata@ccs-labs.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.car2x.plexe.traffic;

import org.car2x.plexe.traffic.HeterogeneousControllerTrafficManager;

simple MisbeTrafficManager extends HeterogeneousControllerTrafficManager {

    parameters:
		bool trafficNoiseAround = default(false);
		int numNoisyVehicles = default(4);
		string noisyvehVType = default("vtypeauto");
		double maxPlatoLength = default(platoonSize * (5 + 2 + 50)); // 5=vehLength, 2=standStillDist, 50="large safetyGap for 150km/h"
		double currentPos = default(maxPlatoLength - (30*platoonSize));
		int lane = default(1);
		volatile double goBackBy = default(uniform(25.0, 35.0));

        @class(plexe::MisbeTrafficManager);
}
