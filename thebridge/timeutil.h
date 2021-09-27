#include <chrono>
#include <iostream>

struct Timer {
    using Clock = std::chrono::steady_clock ;
    using Point = Clock::time_point;
    using Duration = double;

    Timer() = default;
    void set_limit(int lim_ms) { limit = lim_ms; }
    void reset() { m_start = Clock::now(); }
    Duration elapsed_time();
    bool out();

    Point m_start { Clock::now() };
    int limit { std::numeric_limits<int>::max() };
};


inline Timer::Duration Timer::elapsed_time() {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    return duration_cast<milliseconds>(
        Clock::now() - m_start).count();
}

inline bool Timer::out() {
    if (limit == std::numeric_limits<int>::max())
    {
        std::cerr << "WARNING: Timerr queried with unset time limit!"
            << std::endl;
    }
    bool ret = elapsed_time() > limit;
    if (ret)
    {
        std::cerr << "Timer out after "
            << elapsed_time() << " Seconds" << std::endl;
    }
    return ret;
}
