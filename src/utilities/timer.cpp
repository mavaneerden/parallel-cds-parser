/**
 * Source:
 *   https://gist.github.com/mcleary/b0bf4fa88830ff7c882d
 * Description:
 *   Implementation of class for high resolution timer using C++ chrono.
 */

#include "timer.hpp"

void Timer::start()
{
    m_StartTime = std::chrono::system_clock::now();
    m_bRunning = true;
}

void Timer::stop()
{
    m_EndTime = std::chrono::system_clock::now();
    m_bRunning = false;
}

long int Timer::elapsedNanoseconds()
{
    std::chrono::time_point<std::chrono::system_clock> endTime;

    if(m_bRunning)
    {
        endTime = std::chrono::system_clock::now();
    }
    else
    {
        endTime = m_EndTime;
    }

    return std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - m_StartTime).count();
}

double Timer::elapsedMilliseconds()
{
    return (double)elapsedNanoseconds() / 1e6;
}

double Timer::elapsedSeconds()
{
    return elapsedMilliseconds() / 1e3;
}