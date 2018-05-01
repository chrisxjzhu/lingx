#include <lingx/core/event_posted.h>
#include <lingx/core/event.h>
#include <lingx/core/queue.h>
#include <cstddef>

namespace lnx {

Queue  Posted_accept_events;
Queue  Posted_events;

void Post_event(Event* ev, Queue* q) noexcept
{
    if (!ev->posted) {
        ev->posted = 1;
        Queue_insert_tail(q, &ev->queue);
    }
}

void Delete_posted_event(Event* ev) noexcept
{
    ev->posted = 0;
    Queue_remove(&ev->queue);
}

void Event_process_posted(const CyclePtr& /*cycle*/, Queue* posted)
{
    while (!Queue_empty(posted)) {
        Queue* q = Queue_head(posted);
        Event* ev = Queue_data(q, Event, queue);

        Delete_posted_event(ev);

        ev->handler(ev);
    }
}

}
