#ifndef _LINGX_CORE_TIMES_H
#define _LINGX_CORE_TIMES_H

#include <lingx/core/common.h>

namespace lnx {

struct Time {
    long sec;
    long msec;
    int  gmtoff;
};

void Time_init();
void Time_update();
void Timezone_update() noexcept;

extern Time* Cached_time;
extern std::string_view Cached_err_log_time;

/* milliseconds elapsed since epoch, used in event timers */
extern ulong Current_msec;

}

#endif
