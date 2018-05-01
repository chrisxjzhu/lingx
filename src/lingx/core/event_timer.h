#ifndef _LINGX_CORE_EVENT_TIMER_H
#define _LINGX_CORE_EVENT_TIMER_H

#include <lingx/core/common.h>

namespace lnx {

const msec_t TIMER_INFINITE = -1;

msec_t Event_find_timer();
void Event_expire_timers();

}

#endif
