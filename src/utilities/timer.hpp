/**
 * Source:
 *   https://gist.github.com/mcleary/b0bf4fa88830ff7c882d
 * Description:
 *   Definition of class for high resolution timer using C++ chrono.
 */

#include <chrono>

class Timer
{
private:
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    bool                                               m_bRunning = false;
public:
    void start();
    void stop();
    long int elapsedNanoseconds();
    double elapsedMilliseconds();
    double elapsedSeconds();
};