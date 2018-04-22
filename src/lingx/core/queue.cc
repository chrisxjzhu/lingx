#include <lingx/core/queue.h>

namespace lnx {

/*
 * find the middle queue element if the queue has odd number of elements
 * or the first element of the queue's second part otherwise
 */

Queue* Queue_middle(Queue* queue) noexcept
{
    Queue* middle = Queue_head(queue);

    if (middle == Queue_last(queue))
        return middle;

    Queue* next = Queue_head(queue);

    for ( ;; ) {
        middle = Queue_next(middle);

        next = Queue_next(next);

        if (next == Queue_last(queue))
            return middle;

        next = Queue_next(next);

        if (next == Queue_last(queue))
            return middle;
    }
}


/* the stable insertion sort */

void Queue_sort(Queue* queue, int (*cmp)(const Queue*, const Queue*)) noexcept
{

    if (Queue_head(queue) == Queue_last(queue))
        return;

    Queue *q, *prev, *next;

    for (q = Queue_next(q); q != Queue_sentinel(queue); q = next) {

        prev = Queue_prev(q);
        next = Queue_next(q);

        Queue_remove(q);

        do {
            if (cmp(prev, q) <= 0)
                break;

            prev = Queue_prev(prev);

        } while (prev != Queue_sentinel(queue));

        Queue_insert_after(prev, q);
    }
}

}
