#ifndef HEURISTIC_H_
#define HEURISTIC_H_

#include <pybind11/pybind11.h>
#include <pybind11/embed.h> // For embedding the interpreter

namespace py = pybind11;

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "plexe/messages/PlatooningBeacon_m.h"
using CAM = PlatooningBeacon;

class Heuristic {
private:
    // JERK TOLERANCE RANGES FOR TIME-SCALE 1s
    static constexpr double TS = 1; // Time-Scale [s]

    static constexpr double JERK_RSX = 0;          // [m/s/s/s]
    static constexpr double JERK_FSX = 0;          // [m/s/s/s]
    static constexpr double JERK_FDX = 8;          // [m/s/s/s]
    static constexpr double JERK_RDX = 20;         // [m/s/s/s]

    static constexpr double ALPHAs = 0.1;            // 10% tolerance for flat zone
    static constexpr double BETAs = 0.25;            // 25% tolerance for triangular zones
    static constexpr double ALPHAp = 0.2;            // 20% tolerance for flat zone
    static constexpr double BETAp = 0.3;             // 30% tolerance for triangular zones

    static constexpr double SMOOTHING_FACTOR = 0.9;
    static constexpr double MAX_TX_RADIUS = 220;   // [m]
    static constexpr double GPS_UNCERTAINITY = 1;  // [m]

    static constexpr double MAX_DELTA_T = 1.3; // [s]

    static constexpr double MINPOSTOL = 0.5;    // [m]
    static constexpr double MINPOSRANGE = 1;    // [m]
    static constexpr double MINSPEEDTOL = 1;    // [m/s]
    static constexpr double MINSPEEDRANGE = 2;  // [m/s]


public:
    static constexpr int NUM_SCORES = 4;
    static constexpr double WARN_THRESHOLD = 1.0 / NUM_SCORES;
    /**
     * Trapezoidal distribution for jerk, speed, and position
     * @return the plausibility of the beacon
     */
    static double computePlausibility(double value, double range_down, double range_up, double flat_down, double flat_up);

    static double computeJerkScore(const CAM* cam, const CAM* cam_prev, double deltaT, double& jerk);
    static double computeSpeedScore(const CAM* cam, const CAM* cam_prev, double deltaT, double& avgspeed_est_modulo, double& se);
    static double computePosScore(const CAM* cam, const CAM* cam_prev, double deltaT, double& avgspeed_est_modulo, double& pe);
    static double catchRangePlausibilityScore(const CAM* cam, double rxvposx, double rxvposy);

    static double computeWeight(double score);
};

#endif
