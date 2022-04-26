#include "host_timer.h"

HostTimer::HostTimer() {
    reset();
}

void HostTimer::reset() {
    start = clock::now();
}

double HostTimer::elapsed() const {
    auto end = clock::now();
    std::chrono::duration<double> duration = end - start;
    return duration.count();
}