#ifndef _LINGX_CORE_EVENT_H
#define _LINGX_CORE_EVENT_H

#include <lingx/core/common.h>

namespace lnx {

typedef void (*EventHandler)(Event* ev);

struct Event {
    void*     next;

    unsigned  write:1;
    unsigned  accept:1;

    /* used to detect the stale events in kqueue and epoll */
    unsigned  instance:1;

    /* the ready event; in aio mode 0 means that no operation can be posted */
    unsigned  ready:1;

    unsigned  eof:1;
    unsigned  error:1;

    unsigned  pending_eof:1;

    unsigned  closed:1;

    unsigned  available:1;

    EventHandler handler;

    uint      index;
};

struct EventActions {
};

enum {
    USE_GREEDY_EVENT = 0x00000020,
    USE_EPOLL_EVENT  = 0x00000040
};

struct EventConf : MConf {
    uint   connections;
    flag_t multi_accept;
};

struct EventModuleCtx : ModuleCtx {
    typedef std::function<MConfPtr(Cycle*)> create_conf_t;
    typedef std::function<const char*(Cycle*, MConf*)> init_conf_t;

    EventModuleCtx(const char* name,
                   const create_conf_t& create_conf,
                   const init_conf_t& init_conf,
                   const EventActions& actions)
        : name(name), create_conf(create_conf), init_conf(init_conf),
          actions(actions)
    { }

    std::string_view name;
    create_conf_t create_conf;
    init_conf_t init_conf;
    EventActions actions;
};

extern bool Use_epoll_rdhup;
extern int Event_flags;

}

#endif
