#include <lingx/core/event_timer.h>
#include <lingx/core/event.h>
#include <lingx/core/rbtree.h>
#include <lingx/core/times.h>  // Current_msec
#include <cstddef>

namespace lnx {
namespace {

RBTree Event_timer_rbtree_(Rbnode_ptr_cmp);

}

msec_t Event_find_timer()
{
    auto it = Event_timer_rbtree_.cbegin();
    if (it == Event_timer_rbtree_.cend())
        return TIMER_INFINITE;

    signed_msec_t timer = (signed_msec_t) ((*it)->key - Current_msec);

    return (msec_t) (timer > 0 ? timer : 0);
}

void Event_expire_timers()
{
    for ( ;; ) {
        auto it = Event_timer_rbtree_.cbegin();

        if (it == Event_timer_rbtree_.cend())
            return;

        if ((*it)->key > Current_msec)
            return;

        Event* ev = (Event*) ((char*) (*it) - offsetof(Event, timer));

        Event_timer_rbtree_.erase(it);

        ev->timer_set = 0;
        ev->timedout = 1;
        ev->handler(ev);
    }
}

}
