#include <iostream>
#include <string>
#include <omnetpp.h>
#include <vector>
#include <memory>
#include "plexe/PlexeManager.h"
#include <omnetpp.h>
#include <queue>
#include <deque>
#include "veins/modules/mobility/traci/TraCIMobility.h"

#ifndef UTILITIES_H
#define UTILITIES_H

using namespace std;
using namespace omnetpp;
using namespace plexe;

using vehIfi = traci::CommandInterface::Vehicle;
using veinsVehIfi = veins::TraCICommandInterface::Vehicle;

unique_ptr<vehIfi> getPlexeTraciVehicle(string sumoId, PlexeManager* plexe);
veinsVehIfi* getTraciVehicle(string nodeId, cModule* module);
void guiMessage(std::string s);
std::vector<int> convertToTernary(int N, int nVehs);
std::string controllerToString(enum ACTIVE_CONTROLLER ctrl);
enum ACTIVE_CONTROLLER charToController(const char controller);
void computePrediction(double a0, double v0, double p0x, double p0y, simtime_t t0,
    simtime_t t, double angle, double& v, double& px, double& py);

template <typename T, int MaxLen, typename Container = std::deque<T>>
class FixedQueue : public std::queue<T, Container> {
public:
    void push(const T& value)
    {
        if (this->size() == MaxLen) {
            this->c.pop_front();
        }
        std::queue<T, Container>::push(value);
    }
};

#endif