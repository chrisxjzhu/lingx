#ifndef _LINGX_CORE_QUEUE_H
#define _LINGX_CORE_QUEUE_H

namespace lnx {

struct Queue {
    Queue* prev;
    Queue* next;
};

#define Queue_init(q)                                                         \
    (q)->prev = q;                                                            \
    (q)->next = q


#define Queue_empty(h)                                                        \
    (h == (h)->prev)


#define Queue_insert_head(h, x)                                               \
    (x)->next = (h)->next;                                                    \
    (x)->next->prev = x;                                                      \
    (x)->prev = h;                                                            \
    (h)->next = x


#define Queue_insert_after   Queue_insert_head


#define Queue_insert_tail(h, x)                                               \
    (x)->prev = (h)->prev;                                                    \
    (x)->prev->next = x;                                                      \
    (x)->next = h;                                                            \
    (h)->prev = x


#define Queue_head(h)                                                         \
    (h)->next


#define Queue_last(h)                                                         \
    (h)->prev


#define Queue_sentinel(h)                                                     \
    (h)


#define Queue_next(q)                                                         \
    (q)->next


#define Queue_prev(q)                                                         \
    (q)->prev


#define Queue_remove(x)                                                       \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next


#define Queue_split(h, q, n)                                                  \
    (n)->prev = (h)->prev;                                                    \
    (n)->prev->next = n;                                                      \
    (n)->next = q;                                                            \
    (h)->prev = (q)->prev;                                                    \
    (h)->prev->next = h;                                                      \
    (q)->prev = n;


#define Queue_add(h, n)                                                       \
    (h)->prev->next = (n)->next;                                              \
    (n)->next->prev = (h)->prev;                                              \
    (h)->prev = (n)->prev;                                                    \
    (h)->prev->next = h;


#define Queue_data(q, type, link)                                             \
    (type *) ((char *) q - offsetof(type, link))


Queue* Queue_middle(Queue* queue) noexcept;

void Queue_sort(Queue* queue, int (*cmp)(const Queue*, const Queue*)) noexcept;

}

#endif
