#define WANT_WINSOCK2
#include <platdep/sockets.h>
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__) || defined(_WIN64)
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <sstream>
#include <iostream>
#include <fstream>

#include "veins/modules/mobility/traci/TraCIScenarioManagerForker.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"
#include "veins/modules/mobility/traci/TraCILauncher.h"
#include "plexe/traci/PlexeScenarioManagerForker.h"

using veins::TraCILauncher;
// using veins::TraCIScenarioManagerForker;
using plexe::PlexeScenarioManagerForker;

Define_Module(plexe::PlexeScenarioManagerForker);

namespace {

template <typename T>
inline std::string replace(std::string haystack, std::string needle, T newValue)
{
    size_t i = haystack.find(needle, 0);
    if (i == std::string::npos) return haystack;
    std::ostringstream os;
    os << newValue;
    haystack.replace(i, needle.length(), os.str());
    return haystack;
}

} // namespace

PlexeScenarioManagerForker::PlexeScenarioManagerForker()
{
    server = nullptr;
}

PlexeScenarioManagerForker::~PlexeScenarioManagerForker()
{
    killServer();
}

void PlexeScenarioManagerForker::initialize(int stage)
{
    if (stage == 1) {
        commandLine = par("commandLine").stringValue();
        command = par("command").stringValue();
        configFile = par("configFile").stringValue();
        seed = par("seed");
        outputPrefix = par("outputPrefix").stringValue();
        tripInfoOutput = par("tripInfoOutput").stringValue();
        collisionOutput = par("collisionOutput").stringValue();
        stepLength = par("stepLength").doubleValue(); // if not set gets a -1.0 default
        killServer();
    }
    TraCIScenarioManager::initialize(stage);
    if (stage == 1) {
        startServer();
    }
}

void PlexeScenarioManagerForker::finish()
{
    TraCIScenarioManager::finish();
    killServer();
}

void PlexeScenarioManagerForker::startServer()
{
    // autoset seed, if requested
    if (seed == -1) {
        const char* seed_s = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNNUMBER);
        seed = atoi(seed_s);
    }

    // assemble commandLine
    commandLine = replace(commandLine, "$command", command);
    commandLine = replace(commandLine, "$configFile", configFile);
    commandLine = replace(commandLine, "$seed", seed);
    commandLine = replace(commandLine, "$port", port);

    std::stringstream ss;
    if (stepLength > 0) // if not set gets a -1.0 default
        ss << "--step-length " << stepLength;
    commandLine = replace(commandLine, "$stepLength", ss.str());

    ss.str("");
    ss.clear();
    if (outputPrefix.compare("") != 0)
        ss << "--output-prefix " << outputPrefix;
    commandLine = replace(commandLine, "$outputPrefix", ss.str());

    ss.str("");
    ss.clear();
    if (tripInfoOutput.compare("") != 0)
        ss << "--tripinfo-output " << tripInfoOutput << " --tripinfo-output.write-undeparted";
    commandLine = replace(commandLine, "$tripInfoOutput", ss.str());

    // --collision-output collision.xml
    ss.str("");
    ss.clear();
    if (collisionOutput.compare("") != 0)
        ss << "--collision-output " << collisionOutput;
    commandLine = replace(commandLine, "$collisionOutput", ss.str());

    // commandLine = commandLine + " --lateral-resolution 0.8";
    std::cout << "Starting SUMO...\n"
        << commandLine << "\n";
    server = new veins::TraCILauncher(commandLine);
}

void PlexeScenarioManagerForker::killServer()
{
    if (server) {
        delete server;
        server = nullptr;
    }
}

int PlexeScenarioManagerForker::getPortNumber() const
{
    int port = veins::TraCIScenarioManager::getPortNumber();
    if (port != -1) {
        return port;
    }

    // find a free port for the forker if port is still -1

    if (initsocketlibonce() != 0) throw cRuntimeError("Could not init socketlib");

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        throw cRuntimeError("Failed to create socket: %s", strerror(errno));
    }

    struct sockaddr_in serv_addr;
    struct sockaddr* serv_addr_p = (struct sockaddr*) &serv_addr;
    memset(serv_addr_p, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = 0;

    if (::bind(sock, serv_addr_p, sizeof(serv_addr)) < 0) {
        throw cRuntimeError("Failed to bind socket: %s", strerror(errno));
    }

    socklen_t len = sizeof(serv_addr);
    if (getsockname(sock, serv_addr_p, &len) < 0) {
        throw cRuntimeError("Failed to get hostname: %s", strerror(errno));
    }

    port = ntohs(serv_addr.sin_port);

    closesocket(sock);
    return port;
}
