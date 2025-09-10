#include "plexe/utilities/utilities.h"
#include <tuple>

using namespace veins;

unique_ptr<vehIfi> getPlexeTraciVehicle(string sumoId, PlexeManager* plexe)
{
    traci::CommandInterface* plexeTraci;
    unique_ptr<vehIfi> plexeTraciVehicle;
    plexeTraci = plexe->getCommandInterface();
    plexeTraciVehicle.reset(new traci::CommandInterface::Vehicle(plexeTraci, sumoId));
    return plexeTraciVehicle;
}

veins::TraCICommandInterface::Vehicle* getTraciVehicle(string nodeId, cModule* module)
{
    veins::TraCIMobility* mobility;
    veins::TraCICommandInterface* traci;
    veins::TraCICommandInterface::Vehicle* traciVehicle;
    //    std::cout << module->getName() << "\n"
    //              << module->getFullName() << "\n";
    //    return nullptr;
    std::stringstream ss;
    ss << "<root>." << nodeId << ".mobility";
    cModule* mod = module->getModuleByPath(ss.str().c_str());

    if (mod == nullptr)
        throw cRuntimeError("Cannot find module");
    mobility = check_and_cast<veins::TraCIMobility*>(mod);
    traci = mobility->getCommandInterface();
    traciVehicle = mobility->getVehicleCommandInterface();
    return traciVehicle;
}

void guiMessage(std::string s)
{
    char text[128];
    sprintf(text, "%s", s.c_str());
    getSimulation()->getActiveEnvir()->alert(text);
}

std::string controllerToString(enum ACTIVE_CONTROLLER ctrl)
{
    if (ctrl == 0)
        return "DRIVER";
    else if (ctrl == 1)
        return "ACC";
    else if (ctrl == 2)
        return "CACC";
    else if (ctrl == 3)
        return "FAKED_CACC";
    else if (ctrl == 4)
        return "PLOEG";
    else if (ctrl == 5)
        return "CONSENSUS";
    else if (ctrl == 6)
        return "FLATBED";
    else
        throw cRuntimeError("Invalid controller %d", ctrl);
}

enum ACTIVE_CONTROLLER charToController(const char controller)
{
    if (controller == 'A') {
        return ACC;
    }
    else if (controller == 'P') {
        return CACC;
    }
    else if (controller == 'L') {
        return PLOEG;
    }
    else if (controller == 'C') {
        return CONSENSUS;
    }
    else if (controller == 'F') {
        return FLATBED;
    }
    else {
        throw cRuntimeError("Invalid controller selected");
    }
}

void computePrediction(double a0, double v0, double p0x, double p0y, simtime_t t0,
    simtime_t t, double angle, double& v, double& px, double& py)
{
    simtime_t oppdt = t - t0;
    double dt = oppdt.dbl();
    v = v0 + a0 * dt;
    double v0x = v0 * std::cos(angle);
    double v0y = v0 * std::sin(angle);
    double vx = v * std::cos(angle);
    double vy = v * std::sin(angle);
    px = p0x + (v0x + vx) / 2 * dt;
    py = p0y + (v0y + vy) / 2 * dt;
}

/* TEST TERNARY FUNCTION IF YOU WANT :)
 #include <iostream>
 #include <vector>
 #include <cmath>
   using namespace std;

   std::vector<int> convertToTernary(int N, int nVehs)
   {
    std::vector<int> res;
    int quotient = N;

    for (int i = nVehs-1; i > 0; i--) {
        int temp= quotient;
        quotient = temp/3;
        int remainder = temp % 3;
        res.push_back(remainder);
    }

    return res;
   }


   int main()
   {
    double configMax = std::pow(3, (4 - 1)) - 1;

    for (int i=0; i<=configMax; i++) {
        std::vector<int> bau = convertToTernary(i, 4);
        cout << i << ":\t[";
        for(int j : bau)
            cout << " " << j;
        //return 0;
        cout << " ]"<<endl;
        //std::cout << i << ":\t" <<  << std::endl;
    }
    return 0;
   }
 */
