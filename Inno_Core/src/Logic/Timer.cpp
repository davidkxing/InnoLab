#include "Timer.h"

   
int temp = 0;
std::chrono::time_point<std::chrono::steady_clock> start, end;
std::chrono::duration<float> duration;

Timer::Timer() {
    if (temp == 0) {
        timeVal = 20.0f;
        temp += 1;
    }
    start = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;

    if (duration.count() * 1000.0f >= 14.0f && timeVal == 0) {
        timeVal == duration.count() * 1000.0f;
    }
    else {
        timeVal = timeVal + (duration.count() * 1000.0f);
    }
}