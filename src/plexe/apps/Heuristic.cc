#include "plexe/apps/Heuristic.h"

#define PYBIND11_EXPORT __attribute__((visibility("default")))
#include <cassert>

// JERK TOLERANCE RANGES FOR TIME-SCALE 1s
const double Heuristic::TS = 1; // Time-Scale [s]

const double Heuristic::JERK_RSX = 0;          // [m/s/s/s]
const double Heuristic::JERK_FSX = 0;          // [m/s/s/s]
const double Heuristic::JERK_FDX = 8;          // [m/s/s/s]
const double Heuristic::JERK_RDX = 20;         // [m/s/s/s]

const double Heuristic::ALPHAs = 0.1;            // 10% tolerance for flat zone
const double Heuristic::BETAs = 0.25;            // 25% tolerance for triangular zones
const double Heuristic::ALPHAp = 0.2;            // 20% tolerance for flat zone
const double Heuristic::BETAp = 0.3;             // 30% tolerance for triangular zones

const double Heuristic::SMOOTHING_FACTOR = 0.9;
const double Heuristic::MAX_TX_RADIUS = 220;   // [m]
const double Heuristic::GPS_UNCERTAINITY = 1;  // [m]

const double Heuristic::MAX_DELTA_T = 1.3; // [s]

const double Heuristic::MINPOSTOL = 0.5;    // [m]
const double Heuristic::MINPOSRANGE = 1;    // [m]
const double Heuristic::MINSPEEDTOL = 1;    // [m/s]
const double Heuristic::MINSPEEDRANGE = 2;  // [m/s]

const int Heuristic::NUM_SCORES = 4;
const double Heuristic::WARN_THRESHOLD = 1.0 / NUM_SCORES;

double Heuristic::computePlausibility(double value, double rsx, double rdx, double fsx, double fdx) {
    assert(rsx <= fsx && fsx <= fdx && fdx < rdx);

    if (rsx <= value && value < fsx) {
        return (value - rsx) / (fsx - rsx);
    } else if (fsx <= value && value < fdx) {
        return 1;
    } else if (fdx <= value && value < rdx) {
        return (rdx - value) / (rdx - fdx);
    } else {
        return 0;
    }
}

double Heuristic::computeJerkScore(const CAM* cam, const CAM* cam_prev, double deltaT, double& jerk) {
    jerk = std::abs(cam->getAcceleration() - cam_prev->getAcceleration()) / deltaT;  // m/s / s --> m/s^3
    // if small deltaT, allow jerk to be larger
    double sf = (TS / deltaT);  // sf aka scaleFactor, the smaller DeltaT, the larger the sf
    double plau = computePlausibility(jerk, JERK_RSX * sf, JERK_RDX * sf, JERK_FSX * sf, JERK_FDX * sf);
    return 1 - plau;
}

double Heuristic::computeSpeedScore(const CAM* cam, const CAM* cam_prev, double deltaT, double& avgspeed_est_modulo, double& se)
{
    double accX_prev = cam_prev->getAcceleration() * std::cos(cam_prev->getAngle());
    double accY_prev = cam_prev->getAcceleration() * std::sin(cam_prev->getAngle());

    double speedX_est = cam_prev->getSpeedX() + accX_prev * deltaT;
    double speedY_est = cam_prev->getSpeedY() + accY_prev * deltaT;

    se = std::hypot(speedX_est - cam->getSpeedX(), speedY_est - cam->getSpeedY());

    double AEF = (deltaT / TS);
    avgspeed_est_modulo = std::hypot((cam_prev->getSpeedX() + speedX_est)/2, (cam_prev->getSpeedY() + speedY_est)/2);
    double flat_tolerance = ALPHAs * AEF * avgspeed_est_modulo;
    flat_tolerance = std::max(flat_tolerance, MINSPEEDTOL);

    double range_tolerance = BETAs * AEF * avgspeed_est_modulo;
    range_tolerance = std::max(range_tolerance, MINSPEEDRANGE);

    double plau = Heuristic::computePlausibility(se, 0, range_tolerance, 0, flat_tolerance);
    return 1 - plau;
}

double Heuristic::computePosScore(const CAM* cam, const CAM* cam_prev, double deltaT, double& avgspeed_est_modulo, double& pe)
{
    double accX_prev = cam_prev->getAcceleration() * std::cos(cam_prev->getAngle());
    double accY_prev = cam_prev->getAcceleration() * std::sin(cam_prev->getAngle());

    double posX_est = cam_prev->getPositionX() + cam_prev->getSpeedX() * deltaT + 0.5 * accX_prev * pow(deltaT, 2);
    double posY_est = cam_prev->getPositionY() + cam_prev->getSpeedY() * deltaT + 0.5 * accY_prev * pow(deltaT, 2);

    pe = std::hypot(posX_est - cam->getPositionX(), posY_est - cam->getPositionY());

    double AEF = (deltaT / TS);
    double avgmove_est = avgspeed_est_modulo * deltaT;
    double flat_tolerance = ALPHAp * AEF * avgmove_est;
    flat_tolerance = std::max(flat_tolerance, MINPOSTOL);

    double range_tolerance = BETAp * AEF * avgmove_est;
    range_tolerance = std::max(range_tolerance, MINPOSRANGE);

    double plau = Heuristic::computePlausibility(pe, 0, range_tolerance, 0, flat_tolerance);
    return 1 - plau;
}

double Heuristic::catchRangePlausibilityScore(const CAM* cam, double rxvposx, double rxvposy)
{
    if (!Py_IsInitialized()) {
        py::initialize_interpreter();
    }

    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("insert")(0, "./offlineAnalysis");
    py::module_ rules = py::module_::import("rules");

    py::tuple p1 = py::make_tuple(cam->getPositionX(), cam->getPositionY());
    py::tuple p2 = py::make_tuple(rxvposx, rxvposy);

    py::object result = rules.attr("catchRangePlausibilityScore")(p1, p2, MAX_TX_RADIUS, GPS_UNCERTAINITY);

    return 1 - result.cast<double>();
}

double Heuristic::computeWeight(double score)
{
    double min_val = 1.0 / NUM_SCORES;
    double left_knee = 1.0 / (2 * NUM_SCORES);
    double right_knee = (1.0 / NUM_SCORES) + 0.5 * (1 - 1.0 / NUM_SCORES);

    if (score <= left_knee)
        return 1;
    else if (score <= min_val)
        return 1 - ((score - left_knee) / (min_val - left_knee)) * (1 - min_val);
    else if (score <= right_knee)
        return min_val + ((score - min_val) / (right_knee - min_val)) * (1 - min_val);
    else
        return 1;
}
