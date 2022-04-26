#pragma once

/**
    ------ Stolen from the assignments :))
*/

#include <chrono>

class HostTimer {
private:
    using clock = std::chrono::high_resolution_clock;

    clock::time_point start;

public:
    HostTimer();
    void reset();
    double elapsed() const;
};