#include <lingx/core/times.h>
#include <cassert>
#include <cstdio>      // sprintf()
#include <cstring>     // strncmp()
#include <sys/time.h>  // gettimeofday()
#include <time.h>      // gmtime_r(), localtime_r()

namespace {

void Time_init_test()
{
    lnx::Time_init();

    struct timeval tv;
    ::gettimeofday(&tv, nullptr);

    struct tm tm;
    ::localtime_r(&tv.tv_sec, &tm);
    tm.tm_mon++;
    tm.tm_year += 1900;

    char p1[sizeof("1970/09/28 12:00:00")];

    std::sprintf(p1, "%4d/%02d/%02d %02d:%02d:%02d",
                     tm.tm_year, tm.tm_mon, tm.tm_mday,
                     tm.tm_hour, tm.tm_min, tm.tm_sec);

    assert(std::strcmp(lnx::Cached_err_log_time.data(), p1) == 0);
}

}

int main()
{
    Time_init_test();

    return 0;
}
