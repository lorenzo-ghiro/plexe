//
// Copyright (C) 2012-2023 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2023 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#ifndef AIDEFENSEAPP_H_
#define AIDEFENSEAPP_H_

#define NODEF "nodef"
#define HEUADV "heuadv"
#define NNDEF "NNdef"
#define FULL "full"

#define DECIDER_THRESHOLD 0

#include "plexe/apps/NNWrapper.h"
#include "plexe/apps/SimplePlatooningApp.h"
#include "plexe/traffic/MisbeTrafficManager.h"
#include "plexe/utilities/RotationLog.h"
#include "plexe/apps/Heuristic.h"
#include <unordered_map>
#include <vector>

using namespace std;

namespace plexe {

enum DefenseState{
	FOLLOWING,
	GAP_CONTROL,
	AUTONOMOUS
};

struct LOGGING_STRUCT {
    int predictedVeh;
    int mdspl = -1;
    double mdso = -1;
    int lr = -1;
    double cr = -1;
    int lai = -1;
    double cai = -1;
    double muai = -1;
    double CLai = -1;
    int nblai = -1;
    double norm_sum = -1;
    double js = -1;
    double ss = -1;
    double ps = -1;
    double arts = -1;
    double jerk = -1;
    double se = -1;
    double pe = -1;
};

class MisbeProtocol;
class MisbehaviorScenario;

class DefenseApp : public SimplePlatooningApp {
private:
    std::unordered_map<int, RotationLog> beaconBuffer;
	int bufferSize;
	double maxCamAge;
	int bitmask;
	std::map<int, double> smoothed_scores;
	static bool systemIsSafe;


public:
    /** c'tor for GeneralPlatooningApp */
    DefenseApp()
        : currentState(FOLLOWING)
        , updateGapMsg(nullptr)
        , gapControlEnabled(false)
	{
	}

    std::map<std::string, unsigned int> gtMap = {
            {"TN", 0},
            {"FP", 1},
            {"FN", 2},
            {"TP", 3},
            {"exception", 5}
        };

    /** override from BaseApp */
    virtual void initialize(int stage) override;

    /** d'tor for GeneralPlatooningApp */
    virtual ~DefenseApp();

    /** override from BaseApp */
    virtual void handleSelfMsg(cMessage* msg) override;

    int numInitStages() const override { return 3; }

    /**
     * Active an attack type in the protocol
     */
    virtual void activeAttack(const char* type);

    /**
     * Set the index used to the data replay attack
     */
    virtual void setReplayIndex(int index);

protected:


    virtual void beaconBufferManagement(std::unique_ptr<const CAM> cam);
    virtual bool evaluateBeaconPlausibility(const CAM* cam, LOGGING_STRUCT& log);

    // Confidence Score Fusion
    virtual double CSF(LOGGING_STRUCT& ruleLog, LOGGING_STRUCT& aiLog);

    virtual void mergeLogs(LOGGING_STRUCT& log, LOGGING_STRUCT& ruleLog, LOGGING_STRUCT& aiLog);

    /**
     * Handles PlatoonBeacons
     */
    virtual void onPlatoonBeacon(const PlatooningBeacon* pb) override;

    // wrapper for neural network
    NNWrapper* nn_wrapper;

    // actual state in the defense protocol
    DefenseState currentState;

    // update gap with self message
	cMessage* updateGapMsg;

	// traffic manager
	MisbeTrafficManager* traffic;

	MisbehaviorScenario* scenario;

	double h, g, v;
	double g_t, h_t, d_t;

	// gap control is enable
	bool gapControlEnabled;

	double deltaG, deltaT, deltaH;

	// true if the gap has to be increased
	bool increasingGap;

	double misbehaveTime;
	double detectionTime;

	// activate the defense protocol in heuristic base defense, heuristic advanced defense, AI defense, or not
	string defenseEnabled;

	// path to NN model and scaler produced
    string model_path;
    string scaler_path;

    // style for the NN model
    string window_style;

	// activate the use of the radar instead of beacon or not
	bool radar;

	// signal to be emit in vec file that represent the receiver
	simsignal_t aidaIdSignal;

	// signal to be emit in vec file that represent the sender
	simsignal_t senderIdSignal;

	// signal to be emit in vec file that represent the ground truth
	simsignal_t tlSignal;

	// signal to be emit in vec file that represent the final prediction
    simsignal_t mdsplSignal;

	// signal to be emit in vec file that represent the final confidence of the prediction
	simsignal_t mdsoSignal;

	// signal to be emit in vec file that represent the rule label
	simsignal_t lrSignal;

	// signal to be emit in vec file that represent the confidence of the rule label
	simsignal_t crSignal;

	// signal to be emit in vec file that represent the ai label
	simsignal_t laiSignal;

	// signal to be emit in vec file that represent the confidence of the ai label
	simsignal_t caiSignal;

	// signal to be emit in vec file that represent the point estimate of ai label
	simsignal_t muaiSignal;

	// signal to be emit in vec file that represent the confidence level of ai label
	simsignal_t CLaiSignal;

	// signal to be emit in vec file that represent the non binary ai label
	simsignal_t nblaiSignal;

	// signal to be emit in vec file that represent the norm sum metrics of rule mds
	simsignal_t norm_sumSignal;

	// signal to be emit in vec file that represent the jerk score computed by rule mds
	simsignal_t jsSignal;

	// signal to be emit in vec file that represent the speed score computed by rule mds
	simsignal_t ssSignal;

	// signal to be emit in vec file that represent the position score computed by rule mds
	simsignal_t psSignal;

	// signal to be emit in vec file that represent the ART score computed by rule mds
	simsignal_t artsSignal;

	// signal to be emit in vec file that represent the jerk computed by rule mds
	simsignal_t jerkSignal;

	// signal to be emit in vec file that represent the speed error computed by rule mds
	simsignal_t seSignal;

	// signal to be emit in vec file that represent the position error computed by rule mds
	simsignal_t peSignal;

	MisbeProtocol* protocol;

    /**
     * Manage the prediction phase given a beacon
     * @return true if there's an attack or a misbehaviour
     */
    virtual bool managePrediction(const PlatooningBeacon* pb, LOGGING_STRUCT& log);

    virtual void logPrediction(LOGGING_STRUCT& log, int prediction);

    /**
     * Based on the actual state and the prediction
     * and the warning signal it changes the state and
     * other useful variables
     * @param int prediction the prediction of the nn
     * @param bool warning the warning received with a beacon
     */
    void changeState(bool attack, bool warning, int id);

	/**
	 * @return which ctrl has to use the time headway
	 */
	virtual bool usingTimeHeadway();

	/**
	 * Start the gap control
	 * @param enum ACTIVE_CONTROLLER controller the active controller
	 */
    virtual void startGapControl(enum ACTIVE_CONTROLLER controller);

    /**
     * Update the gap every self msg
     */
    virtual void updateGap();

    /**
     * Set the controller headeway and distance over
     * a gap reduction
     */
    virtual void setControllerGap(double h, double d);

    /**
     * @return if the gap control is completed
     */
    virtual bool isGapControlCompleted();

    /**
     * @return if the gap is reached
     */
    virtual bool isGapReached();

    /**
     * Manage when the gap is reached
     * and do the downgrade to ACC
     */
    virtual void gapReached();

    /**
     * Set the warning value of sent beacons
     * when a misbehaviour is detected
     * @param bool warning the warning signal
     */
    void setWarning(bool warning);
};

} // mamespace plexe

#endif
