//
// Copyright (C) 2012-2021 Michele Segata <segata@ccs-labs.org>
// Copyright (C) 2018-2021 Julian Heinovski <julian.heinovski@ccs-labs.org>
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

#include "DefenseApp.h"
#include "plexe/scenarios/MisbehaviorScenario.h"

#include "plexe/protocols/MisbeProtocol.h"

#include <math.h>
#include <cmath>

using namespace veins;

namespace plexe {

Define_Module(DefenseApp);

bool DefenseApp::systemIsSafe = true;

void DefenseApp::initialize(int stage)
{
    SimplePlatooningApp::initialize(stage);

    if(stage == 0){
        defenseEnabled = par("defenseEnabled").stringValue();
        window_style = par("window_style").stringValue();
        model_path = par("model_path").stringValue();
        scaler_path = par("scaler_path").stringValue();
        radar = par("radar").boolValue();
        bufferSize = par("bufferSize").intValue();
        maxCamAge = par("maxCamAge").doubleValueInUnit("s");
        bitmask = static_cast<uint8_t>(par("bitmask").intValue());

        aidaIdSignal = registerSignal("aidaId");
        senderIdSignal = registerSignal("senderId");
        tlSignal = registerSignal("tl");
        mdsplSignal = registerSignal("mdspl");
        mdsoSignal = registerSignal("mdso");
        lrSignal = registerSignal("lr");
        crSignal = registerSignal("cr");
        laiSignal = registerSignal("lai");
        caiSignal = registerSignal("cai");
        muaiSignal = registerSignal("muai");
        CLaiSignal = registerSignal("CLai");
        nblaiSignal = registerSignal("nblai");
        norm_sumSignal = registerSignal("norm_sum");
        jsSignal = registerSignal("js");
        ssSignal = registerSignal("ss");
        psSignal = registerSignal("ps");
        artsSignal = registerSignal("arts");
        jerkSignal = registerSignal("jerk");
        seSignal = registerSignal("se");
        peSignal = registerSignal("pe");
    }

    if(stage == 1){
        protocol = FindModule<MisbeProtocol*>::findSubModule(getParentModule());
        traffic = FindModule<MisbeTrafficManager*>::findGlobalModule();
        plexeTraciVehicle->useRadar(radar);
        protocol->setWarning(false);
        if (defenseEnabled == NNDEF or defenseEnabled == FULL)
            nn_wrapper = new NNWrapper(model_path, scaler_path, par("MCdropRep").intValue(), par("MCdropCU").doubleValue());
        misbehaveTime = traffic->par("timeMisbehavior").doubleValue();
    }

    if(stage == 2) {
        scenario = FindModule<MisbehaviorScenario*>::findSubModule(getParentModule());
    }
}

DefenseApp::~DefenseApp()
{
    cancelAndDelete(updateGapMsg);
    if (defenseEnabled == NNDEF or defenseEnabled == FULL)
        delete nn_wrapper;
}

void DefenseApp::handleSelfMsg(cMessage* msg)
{
    if (msg == updateGapMsg) {
        updateGap();
        return;
    }
    BaseApp::handleSelfMsg(msg);
}

void DefenseApp::startGapControl(enum ACTIVE_CONTROLLER controller)
{
    this->h_t = 1.2;
    this->d_t = 2;
    double rv, currentDistance;
    plexeTraciVehicle->getRadarMeasurements(currentDistance, rv);

    g = currentDistance;
    v = traciVehicle->getSpeed();

    deltaT = 0.1; // deltaT is the time interval with which update the gap
    deltaG = 1; // m / s
    deltaH = deltaG / v; // s/s = #

    g_t = h_t * v + d_t;

    if (usingTimeHeadway()) {
        g = d_t;
        h = (currentDistance - d_t) / v;
        if (h_t < h) increasingGap = false;
        else increasingGap = true;
    }
    else {
        g = currentDistance;
        h = 0;
        if (g_t < g) increasingGap = false;
        else increasingGap = true;
    }
    if (!updateGapMsg)
        updateGapMsg = new cMessage("updateGap");
    if (!gapControlEnabled)
        scheduleAt(simTime(), updateGapMsg);
    gapControlEnabled = true;
}

void DefenseApp::updateGap()
{
    v = traciVehicle->getSpeed();
    deltaH = deltaG / v;
    g_t = h_t * v + d_t;
    if (isGapControlCompleted()) {
        if (usingTimeHeadway()) {
            h = h_t;
        }
        else {
            g = g_t;
        }
        setControllerGap(h, g);
        if (isGapReached()) {
            gapControlEnabled = false;
            gapReached();
            return;
        }
    }
    else {
        if (usingTimeHeadway()) {
            h = h + (h_t - h > 0 ? 1 : -1) * deltaH * deltaT;
        }
        else {
            g = g + (g_t - g > 0 ? 1 : -1) * deltaG * deltaT;
        }
        setControllerGap(h, g);
    }
    scheduleAt(simTime() + deltaT, updateGapMsg);
}

void DefenseApp::setControllerGap(double h, double d)
{
    plexeTraciVehicle->setCACCConstantSpacing(d);
    plexeTraciVehicle->setPloegCACCParameters(-1, -1, h);
    plexeTraciVehicle->setACCHeadwayTime(h);
}

bool DefenseApp::isGapControlCompleted()
{
    if (usingTimeHeadway()) {
        if ((increasingGap && h >= h_t) || (!increasingGap && h <= h_t))
            return true;
    }
    else {
        if ((increasingGap && g >= g_t) || (!increasingGap && g <= g_t))
            return true;
    }
    return false;
}

bool DefenseApp::isGapReached()
{
    double rv, curDistance;
    plexeTraciVehicle->getRadarMeasurements(curDistance, rv);
    if ((increasingGap && curDistance >= g_t) || (!increasingGap && curDistance <= g_t))
        return true;
    return false;
}

void DefenseApp::gapReached()
{
    plexeTraciVehicle->setActiveController(1);
    setControllerGap(1.2, 2);
    currentState = AUTONOMOUS;
}


bool DefenseApp::usingTimeHeadway()
{
    switch (plexeTraciVehicle->getActiveController()) {
    case plexe::CACC:
        return false;
        break;
    case plexe::PLOEG:
    case plexe::ACC:
        return true;
        break;
    default:
        throw new cRuntimeError("Undefined");
        break;
    }
}

bool DefenseApp::managePrediction(const CAM* cam, LOGGING_STRUCT& log)
{
	int id = cam->getVehicleId();
	auto& beaconLog = beaconBuffer[id];
	bool work = (beaconLog.size() == 5);
	if (work) {
	    std::vector<const CAM*> ordMsgs = beaconLog.getOrderedMessages();
	    log.nblai = nn_wrapper->predict(ordMsgs, log.nblai, log.cai, log.muai, log.CLai);
		log.lai = (log.nblai == 0) ? 0 : 1;
		if (defenseEnabled != FULL)
		    logPrediction(log, log.nblai);
		if (log.nblai != 0 and log.nblai != -1 and defenseEnabled == NNDEF){
		    detectionTime = simTime().dbl();
		    traffic->setReactionTime(detectionTime - misbehaveTime);
			return true;
		} else {
			return false;
		}
	}

	if (defenseEnabled != FULL)
	    logPrediction(log, log.nblai);
    return false;
}

void DefenseApp::logPrediction(LOGGING_STRUCT& log, int prediction)
{
    emit(aidaIdSignal, myId);
    emit(senderIdSignal, log.predictedVeh);

    double misbTime = traffic->getTimeMisbehavior();

    // determine if TP, TN, FP, FN
    // case negative == genuine
    string groundTruth;
    bool afterAttack = simTime().dbl() >= misbTime;
    bool targetVehIsTheAttacker = log.predictedVeh == scenario->getIdMisbehavior();
    bool trueAttack = afterAttack and targetVehIsTheAttacker;
    if (prediction == 0) {
        if (trueAttack)
            groundTruth = "FN";
        else
            groundTruth = "TN";
    }
    else if (prediction > 0) {
        if (trueAttack)
            groundTruth = "TP";
        else
            groundTruth = "FP";
    }
    else {
        groundTruth = "exception";
    }

    emit(tlSignal, gtMap[groundTruth]);

    emit(mdsplSignal, log.mdspl);
    emit(mdsoSignal, log.mdso);
    emit(lrSignal, log.lr);
    emit(crSignal, log.cr);
    emit(laiSignal, log.lai);
    emit(caiSignal, log.cai);
    emit(muaiSignal, log.muai);
    emit(CLaiSignal, log.CLai);
    emit(nblaiSignal, log.nblai);
    emit(norm_sumSignal, log.norm_sum);
    emit(jsSignal, log.js);
    emit(ssSignal, log.ss);
    emit(psSignal, log.ps);
    emit(artsSignal, log.arts);
    emit(jerkSignal, log.jerk);
    emit(seSignal, log.se);
    emit(peSignal, log.pe);
}


bool DefenseApp::evaluateBeaconPlausibility(const CAM* cam, LOGGING_STRUCT& log)
{
    // beacon sender id
    int id = cam->getVehicleId();

    if (beaconBuffer[id].size() < 2) {
        if (defenseEnabled != FULL)
            logPrediction(log, log.lr); // we dont have enough beacons, emit -1 to say "no prediction"
        return false;
    }

    // we have enough info, so we perform all checks
    // previous beacon
    auto cam_prev = beaconBuffer[id].previous();

    // delta time
    double time = cam->getTime();
    double time_prev = cam_prev->getTime();
    double deltaT = time - time_prev;

    // Compute jerk, speed and pos scores
    double avgspeed_est_modulo = 0;
    double jerk = 0;
    double se = 0;
    double pe = 0;
    log.js = Heuristic::computeJerkScore(cam, cam_prev, deltaT, jerk);
    log.ss = Heuristic::computeSpeedScore(cam, cam_prev, deltaT, avgspeed_est_modulo, se);
    log.ps = Heuristic::computePosScore(cam, cam_prev, deltaT, avgspeed_est_modulo, pe);

    log.jerk = jerk;
    log.se = se;
    log.pe = pe;

    VEHICLE_DATA data;
    plexeTraciVehicle->getVehicleData(&data);
    log.arts = Heuristic::catchRangePlausibilityScore(cam, data.positionX, data.positionY);

    double additive_score = log.js + log.ss + log.ps + log.arts;
    log.norm_sum = additive_score / Heuristic::NUM_SCORES;

    log.lr = (log.norm_sum >= Heuristic::WARN_THRESHOLD) ? 1 : 0;
    log.cr = Heuristic::computeWeight(log.norm_sum);

    if (log.lr != 0 && defenseEnabled == HEUADV) {
        detectionTime = simTime().dbl();
        traffic->setReactionTime(detectionTime - misbehaveTime);
    }

    if (defenseEnabled != FULL)
        logPrediction(log, log.lr);
    return log.lr > 0;
}

void DefenseApp::changeState(bool attack, bool warning, int id){
    if(currentState == FOLLOWING and (attack or warning)){
        getSimulation()->getActiveEnvir()->alert("ATTACK!");
        if (positionHelper->isInSamePlatoon(id)) {
            currentState = GAP_CONTROL;
            setWarning(true);
            radar = true;
            plexeTraciVehicle->useRadar(radar);
            startGapControl(static_cast<ACTIVE_CONTROLLER>(plexeTraciVehicle->getActiveController()));
        }
        else
            systemIsSafe = false;
    }
}

void DefenseApp::beaconBufferManagement(std::unique_ptr<const CAM> cam)
{
    int vehicleId = cam->getVehicleId();
    //check if we know already this veh
    bool knownVeh = beaconBuffer.find(vehicleId) != beaconBuffer.end();
    if (!knownVeh)
        beaconBuffer.emplace(vehicleId, std::move(RotationLog(bufferSize)));

    auto& beaconLog = beaconBuffer[vehicleId];
    // garbage-collection
    beaconLog.cleanOldMessages(simTime().dbl(), maxCamAge);

    beaconLog.push(std::move(cam));
}

double DefenseApp::CSF(LOGGING_STRUCT& ruleLog, LOGGING_STRUCT& aiLog)
{
    double rterm = ruleLog.cr * (2 * ruleLog.lr - 1);
    double aiterm = aiLog.cai * (2 * aiLog.lai - 1);

    return rterm + aiterm;
}

void DefenseApp::mergeLogs(LOGGING_STRUCT& log, LOGGING_STRUCT& ruleLog, LOGGING_STRUCT& aiLog)
{
    log.lr = ruleLog.lr;
    log.cr = ruleLog.cr;
    log.lai = aiLog.lai;
    log.cai = aiLog.cai;
    log.muai = aiLog.muai;
    log.CLai = aiLog.CLai;
    log.nblai = aiLog.nblai;
    log.norm_sum = ruleLog.norm_sum;
    log.js = ruleLog.js;
    log.ss = ruleLog.ss;
    log.ps = ruleLog.ps;
    log.arts = ruleLog.arts;
    log.jerk = ruleLog.jerk;
    log.se = ruleLog.se;
    log.pe = ruleLog.pe;
}

void DefenseApp::onPlatoonBeacon(const CAM* cam)
{
    beaconBufferManagement(std::unique_ptr<const CAM>(cam->dup()));

    // determine if this is an attacking beacon or not
    bool attack = false;

    // struct for logging all the needed data
    LOGGING_STRUCT log, ruleLog, aiLog;

    log.predictedVeh = cam->getVehicleId();

    if (currentState == FOLLOWING and systemIsSafe) {
        // dont run any evaluation until you have at least a 5long full window of recent CAMs
        int id = cam->getVehicleId();
        auto& beaconLog = beaconBuffer[id];
        bool needEval = (beaconLog.size() == 5);
        if (needEval) {
            if (defenseEnabled == NODEF) {  // no defense
                logPrediction(log, log.mdspl);
            }
            else if (defenseEnabled == HEUADV) {  // RULE-based defense
                attack = evaluateBeaconPlausibility(cam, log);
            }
            else if (defenseEnabled == NNDEF) {  // AI-based defense
                attack = managePrediction(cam, log);
            }
            else if (defenseEnabled == FULL) {  // Hybrid defense
                bool heu_attack = evaluateBeaconPlausibility(cam, ruleLog);
                bool ai_attack = managePrediction(cam, aiLog);
                // Confidence-based Score Fusion
                log.mdso = CSF(ruleLog, aiLog);
                log.mdspl = (log.mdso > DECIDER_THRESHOLD) ? 1 : 0;

                if (log.mdspl != 0) {
                    detectionTime = simTime().dbl();
                    traffic->setReactionTime(detectionTime - misbehaveTime);
                }

                attack = log.mdspl > 0;

                mergeLogs(log, ruleLog, aiLog);
                logPrediction(log, log.mdspl);
            }
        }
    }

    changeState(attack, cam->getWarning(), cam->getVehicleId()); // see the warning broadcasted
    // set the message to be replayed
    protocol->setReplayMessage(cam);

    SimplePlatooningApp::onPlatoonBeacon(cam);
}

void DefenseApp::setWarning(bool warning)
{
    protocol->setWarning(warning);
}

void DefenseApp::activeAttack(const char* type)
{
    protocol->activeAttack(type);
}

void DefenseApp::setReplayIndex(int index)
{
    protocol->setReplayIndex(index);
}

} //end namespace plexe
