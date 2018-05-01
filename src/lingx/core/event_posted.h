#ifndef _LINGX_CORE_EVENT_POSTED_H
#define _LINGX_CORE_EVENT_POSTED_H

#include <lingx/core/common.h>

namespace lnx {

struct Queue;

void Post_event(Event* ev, Queue* q) noexcept;
void Delete_posted_event(Event* ev) noexcept;

void Event_process_posted(const CyclePtr& cycle, Queue* posted);

extern Queue  Posted_accept_events;
extern Queue  Posted_events;

}

#endif
