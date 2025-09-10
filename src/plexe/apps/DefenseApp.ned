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

package org.car2x.plexe.apps;

import org.car2x.plexe.apps.SimplePlatooningApp;

simple DefenseApp extends SimplePlatooningApp
{
    parameters:
        string defenseEnabled;
        bool radar = default(false);
        int bufferSize = default(5);
        int bitmask = default(0);
        string window_style;
        string model_path;
        string scaler_path;
        int MCdropRep = default(10);
        double MCdropCU = default(0.1);
        double smoothingFactor = default(0.5);
        double maxCamAge @unit("s") = default(1 s);

        @signal[aidaId](type="long");
        @statistic[aidaId](source="aidaId"; record=vector);

        @signal[senderId](type="int");
        @statistic[senderId](source="senderId"; record=vector,stats?,timeavg?);

        @signal[tl](type="unsigned int");
        @statistic[tl](source="tl"; record=vector);

        @signal[mdspl](type="int");
        @statistic[mdspl](source="mdspl"; record=vector,stats?,timeavg?);

        @signal[mdso](type="double");
        @statistic[mdso](source="mdso"; record=vector,stats?,timeavg?);

        @signal[lr](type="int");
        @statistic[lr](source="lr"; record=vector,stats?,timeavg?);

        @signal[cr](type="double");
        @statistic[cr](source="cr"; record=vector,stats?,timeavg?);

        @signal[lai](type="int");
        @statistic[lai](source="lai"; record=vector,stats?,timeavg?);

        @signal[cai](type="double");
        @statistic[cai](source="cai"; record=vector,stats?,timeavg?);

        @signal[muai](type="double");
        @statistic[muai](source="muai"; record=vector,stats?,timeavg?);

        @signal[CLai](type="double");
        @statistic[CLai](source="CLai"; record=vector,stats?,timeavg?);

        @signal[nblai](type="int");
        @statistic[nblai](source="nblai"; record=vector,stats?,timeavg?);

        @signal[norm_sum](type="double");
        @statistic[norm_sum](source="norm_sum"; record=vector,stats?,timeavg?);

		@signal[js](type="double");
        @statistic[js](source="js"; record=vector,stats?,timeavg?);

        @signal[ss](type="double");
        @statistic[ss](source="ss"; record=vector,stats?,timeavg?);

        @signal[ps](type="double");
        @statistic[ps](source="ps"; record=vector,stats?,timeavg?);

        @signal[arts](type="double");
        @statistic[arts](source="arts"; record=vector,stats?,timeavg?);

        @signal[jerk](type="double");
        @statistic[jerk](source="jerk"; record=vector,stats?,timeavg?);

        @signal[se](type="double");
        @statistic[se](source="se"; record=vector,stats?,timeavg?);

        @signal[pe](type="double");
        @statistic[pe](source="pe"; record=vector,stats?,timeavg?);

        @display("i=block/app2");
        @class(plexe::DefenseApp);
}