#ifndef NNWRAPPER_H
#define NNWRAPPER_H

#include <pybind11/pybind11.h>
#include <pybind11/embed.h> // For embedding the interpreter
#include <pybind11/numpy.h> // For numpy array handling
#include <pybind11/stl.h>   // For automatic conversion between STL and Python types

namespace py = pybind11;

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <map>
#include <array>
#include <queue>
#include "plexe/messages/PlatooningBeacon_m.h"

using CAM = PlatooningBeacon;

// Define a structure to hold message data
struct Message {
    double time;
    double posx;
    double posy;
    double spdx;
    double spdy;
    double acl;

    // Default constructor
    Message() : time(0), posx(0), posy(0), spdx(0), spdy(0), acl(0) {}

    // Constructor to initialize the struct
    Message(double t, double x, double y, double spx, double spy, double a)
        : time(t), posx(x), posy(y), spdx(spx), spdy(spy), acl(a) {}

    Message(const CAM* cam)
            : time(cam->getTime()), posx(cam->getPositionX()), posy(cam->getPositionY()), spdx(cam->getSpeedX()), spdy(cam->getSpeedY()), acl(cam->getAcceleration()) {}
};


class NNWrapper {
public:
    NNWrapper(const std::string& model_path, const std::string& scaler_path, int MCdropRep, double MCdropCU);
    ~NNWrapper();
    py::array_t<double> reshapeNormDiff(std::vector<double>& normunshaped);
    int predict(std::vector<const CAM*> ordMsgs, int& label, double& cai, double& muai, double& CLai);

    void predict_with_confidence(py::object& aimds_instance, py::array_t<double>& tensor, int& label, double& mu_selected, double& CL);

    py::array_t<double> normalize(std::vector<const CAM*> rawSequence);

private:

    struct Impl;
    Impl* pimpl;
};

#endif // NNWRAPPER_H
