#ifndef _LINGX_CORE_EVENT_POSTED_H
#define _LINGX_CORE_EVENT_POSTED_H

#include <lingx/core/common.h>

namespace lnx {

struct Queue;

void Post_event(Event* ev, Queue* q) noexcept;

extern Queue  Posted_accept_events;
extern Queue  Posted_events;

}

#endif
