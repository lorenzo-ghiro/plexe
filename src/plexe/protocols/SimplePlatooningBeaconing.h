//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
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

#ifndef SIMPLEPLATOONINGBEACONING_H_
#define SIMPLEPLATOONINGBEACONING_H_

#include "BaseProtocol.h"
#include <random>

namespace plexe {

class SimplePlatooningBeaconing : public BaseProtocol {
protected:
    virtual void handleSelfMsg(cMessage* msg);

    // bound selected for the random and offset position
    double lower_bound_random = 0;
    double upper_bound_random = 10000;
    double lower_bound_offset = -10;
    double upper_bound_offset = 10;

    // bound selected for the random and offset speed
    double lower_bound_random_speed = -200;
    double upper_bound_random_speed = 200;
    double lower_bound_offset_speed = -8;
    double upper_bound_offset_speed = 8;

public:
    SimplePlatooningBeaconing();
    virtual ~SimplePlatooningBeaconing();

    virtual void initialize(int stage);
};

} // namespace plexe

#endif /* SIMPLEPLATOONINGBEACONING_H_ */
