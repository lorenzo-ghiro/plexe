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

import org.car2x.plexe.mobility.BTraCIBaseTrafficManager;

simple HeterogeneousControllerTrafficManager extends BTraCIBaseTrafficManager {

    parameters:
        //platoon insertion time
        double platoonInsertTime @unit("s");
        //platoon insertion speed
        double platoonInsertSpeed @unit("kmph");
        //how many vehicles to inject
        int nCars;
        //platoon size
        int platoonSize;
        //number of lanes
        int nLanes;
        //sumo vehicle type for platooning cars
        string platooningVType;
        //insert distance and headway. distance is computed as:
        //dist = insertHeadway * insertSpeed + insertDistance
        double platoonLeaderHeadway @unit("s") = default(1.2s);
        double platoonAdditionalDistance @unit("m") = default(0m);

        controller = default("APPPPPPP");
        //when using different controllers for different vehicles in a platoon, it is necessary to specify the spacings for each vehicle
        //this is done using the following parameters which should indicate, for each controller, the spacing and the headway
        //the format is the following:
        //platoonDistances="ACC=2 CACC=5 PLOEG=2"
        //platoonHeadways="ACC=1.2 CACC=0 PLOEG=0.5"
        //these parameters are considered only if non empty
        string platoonDistances = default("");
        string platoonHeadways = default("");
        bool createSubPlatoons = default(false);
        //if set to true, assigns a random controller to all the members of the platoon but the leader
        //(which is always assigned an ACC)
        bool useRandomControllerAssignment = default(false);

        double timeMisbehavior;
        double reactionTime = default(-1.0);

        @class(plexe::HeterogeneousControllerTrafficManager);
}
