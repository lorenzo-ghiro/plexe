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
    static const double TS; // Time-Scale [s]

    static const double JERK_RSX;          // [m/s/s/s]
    static const double JERK_FSX;          // [m/s/s/s]
    static const double JERK_FDX;          // [m/s/s/s]
    static const double JERK_RDX;          // [m/s/s/s]

    static const double ALPHAs;            // 10% tolerance for flat zone
    static const double BETAs;             // 25% tolerance for triangular zones
    static const double ALPHAp;            // 20% tolerance for flat zone
    static const double BETAp;             // 30% tolerance for triangular zones

    static const double SMOOTHING_FACTOR;
    static const double MAX_TX_RADIUS;     // [m]
    static const double GPS_UNCERTAINITY;  // [m]

    static const double MAX_DELTA_T;    // [s]

    static const double MINPOSTOL;      // [m]
    static const double MINPOSRANGE;    // [m]
    static const double MINSPEEDTOL;    // [m/s]
    static const double MINSPEEDRANGE;  // [m/s]


public:
    static const int NUM_SCORES;
    static const double WARN_THRESHOLD;
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
