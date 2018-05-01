#include <lingx/core/event.h>
#include <type_traits>
#include <cassert>
#include <cstddef>  // offsetof()

namespace {

void Event_timer_rbtree_test()
{
    using namespace lnx;

    assert(std::is_standard_layout<Event>::value);

    Event ev1;
    Event ev2;
    Event ev3;

    ev1.timer.key = 3;
    ev2.timer.key = 1;
    ev3.timer.key = 2;

    RBTree tree(Rbnode_ptr_cmp);

    tree.insert(&ev1.timer);
    tree.insert(&ev2.timer);
    tree.insert(&ev3.timer);

    rbkey_t k = 1;

    for (auto it = tree.cbegin(); it != tree.cend(); ++it) {
        Event* ev = (Event*) ((char*) (*it) - offsetof(Event, timer));
        assert(ev->timer.key == k);
        k++;
    }
}

}

int main()
{
    Event_timer_rbtree_test();

    return 0;
}
