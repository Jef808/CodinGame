#include <chrono>
#include <iostream>

struct Timer {
    using Clock = std::chrono::steady_clock;
    using Point = Clock::time_point;
    using Duration = double;

    Timer() = default;
    void set_limit(int lim_ms) { limit = lim_ms; }
    void reset() { m_start = Clock::now(); }
    Duration elapsed();
    bool out();

    Point m_start { Clock::now() };
    int limit { std::numeric_limits<int>::max() };
    bool warning_sent { false };
};

inline Timer::Duration Timer::elapsed()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    return duration_cast<milliseconds>(
        Clock::now() - m_start)
        .count();
}

inline bool Timer::out()
{
    if (limit == std::numeric_limits<int>::max() && !warning_sent) {
        warning_sent = true;
        std::cerr << "Timer: Received timeout query but time limit is not set."
                  << std::endl;
    }

    return elapsed() > limit;
}
