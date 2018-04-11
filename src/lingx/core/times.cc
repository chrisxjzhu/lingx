#include <lingx/core/times.h>
#include <sys/time.h>  // gettimeofday()
#include <time.h>      // gmtime_r(), localtime_r()
#include <cstdio>      // sprintf()
#include <mutex>

namespace lnx {

namespace {

const uint TIME_SLOTS_ = 64;

uint Slot_ = 0;

int Cached_gmtoff_ = 0;

Time Cached_time_[TIME_SLOTS_];

char Cached_err_log_time_[TIME_SLOTS_][sizeof("1970/09/28 12:00:00")];

std::mutex Mutex_;

}

ulong Current_msec = 0;

Time* Cached_time = nullptr;

std::string_view Cached_err_log_time;

void Time_init()
{
    Cached_time = &Cached_time_[0];

    Time_update();
}

void Time_update()
{
    if (!Mutex_.try_lock())
        return;

    std::lock_guard<std::mutex> lock(Mutex_, std::adopt_lock);

    struct timeval tv;
    ::gettimeofday(&tv, nullptr);

    long sec = tv.tv_sec;
    long msec = tv.tv_usec / 1000;

    Current_msec = sec * 1000 + msec;

    Time* tp = &Cached_time_[Slot_];

    if (tp->sec == sec) {
        tp->msec = msec;
        return;
    }

    Slot_ = (Slot_ + 1) % TIME_SLOTS_;

    tp = &Cached_time_[Slot_];

    tp->sec = sec;
    tp->msec = msec;

    struct tm gmt;
    ::gmtime_r(&sec, &gmt);

    struct tm tm;
    ::localtime_r(&sec, &tm);
    tm.tm_mon++;
    tm.tm_year += 1900;

    Cached_gmtoff_ = tm.tm_gmtoff / 60;

    tp->gmtoff = Cached_gmtoff_;

    char* p1 = &Cached_err_log_time_[Slot_][0];

    std::sprintf(p1, "%4d/%02d/%02d %02d:%02d:%02d",
                     tm.tm_year, tm.tm_mon, tm.tm_mday,
                     tm.tm_hour, tm.tm_min, tm.tm_sec);

    Cached_time = tp;
    Cached_err_log_time = p1;
}

/*
 * Linux does not test /etc/localtime change in localtime(),
 * but may stat("/etc/localtime") several times in every strftime(),
 * therefore we use it to update timezone.
 */
void Timezone_update() noexcept
{
    time_t t = ::time(0);
    struct tm* tm = ::localtime(&t);
    char buf[4];
    ::strftime(buf, 4, "%H", tm);
}

}
