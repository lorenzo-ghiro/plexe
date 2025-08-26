#ifndef ROTATIONLOG_H
#define ROTATIONLOG_H

#include <deque>
#include <stdexcept>
#include <algorithm>
#include "plexe/messages/PlatooningBeacon_m.h"


namespace plexe {

using CAM = PlatooningBeacon;

class RotationLog {
    int maxSize;
    std::deque<std::unique_ptr<const CAM>> log;

public:
    // Constructor
    RotationLog(int maxSize) : maxSize(maxSize) {
        if (maxSize < 2) {
            throw std::invalid_argument("maxSize must be >= 2 for comparison.");
        }
    }

    RotationLog() : maxSize(5) {}

    ~RotationLog() = default;

    // disable copying
    RotationLog(const RotationLog&) = delete;
    RotationLog& operator=(const RotationLog&) = delete;

    // enable move
    RotationLog(RotationLog&&) = default;
    RotationLog& operator=(RotationLog&&) = default;


    // Add a new CAM to the log
    void push(std::unique_ptr<const CAM> message) {
        if (log.size() == maxSize) {
            log.pop_front();
        }
        log.push_back(std::move(message));
    }

    // Remove old CAMs based on their timestamp
    void cleanOldMessages(double currentTime, double maxAge) {
        auto old = std::remove_if(log.begin(), log.end(),
                [currentTime, maxAge](const std::unique_ptr<const CAM>& cam) {
                    return (currentTime - cam->getTime()) > maxAge;
                });

        log.erase(old, log.end());
    }

    void clear() {
        log.clear();
    }


    // Accessors
    const CAM* latest() const {
        if (log.empty()) {
            throw std::runtime_error("Log is empty.");
        }
        return log.back().get();
    }

    const CAM* previous() const {
        if (log.size() < 2) {
            throw std::runtime_error("Not enough messages.");
        }
        return log[log.size() - 2].get();
    }

    size_t size() const {
        return log.size();
    }

    // Iterators
    auto begin() const { return log.begin(); }
    auto end() const { return log.end(); }

    std::vector<const CAM*> getOrderedMessages() const {
        std::vector<const CAM*> result;
        for (const auto& ptr : log)
            result.push_back(ptr.get());
        return result;
    }
};

} // mamespace plexe

#endif
