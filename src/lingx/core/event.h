#ifndef _LINGX_CORE_EVENT_H
#define _LINGX_CORE_EVENT_H

#include <lingx/core/common.h>
#include <lingx/core/os_io.h>
#include <lingx/core/queue.h>
#include <csignal>

namespace lnx {

const uint INVALID_INDEX = 0xd0d0d0d0;

typedef void (*Event_handler_pt)(Event* ev);

struct Event {
    void*     data;

    unsigned  write:1;
    unsigned  accept:1;

    /* used to detect the stale events in kqueue and epoll */
    unsigned  instance:1;

    unsigned  active:1;

    unsigned  disabled:1;

    /* the ready event; in aio mode 0 means that no operation can be posted */
    unsigned  ready:1;

    unsigned  oneshot:1;

    /* aio operation is complete */
    unsigned  complete:1;

    unsigned  eof:1;
    unsigned  error:1;

    unsigned  pending_eof:1;

    unsigned  posted:1;

    unsigned  closed:1;

    unsigned  available:1;

    Event_handler_pt handler;

    uint  index;
    Log*  log;

    /* the posted queue */
    Queue  queue;
};

struct EventActions {
    rc_t  (*add)(Event* ev, int event, int flags);
    rc_t  (*del)(Event* ev, int event, int flags);

    rc_t  (*enable)(Event* ev, int event, int flags);
    rc_t  (*disable)(Event* ev, int event, int flags);

    rc_t  (*add_conn)(Connection* c);
    rc_t  (*del_conn)(Connection* c, int flags);

    rc_t  (*notify)(Event_handler_pt handler);

    rc_t  (*process_events)(Cycle* cycle, int timer, int flags);

    rc_t  (*init)(Cycle* cycle, int timer);
    void  (*done)(Cycle* cycle);
};

extern EventActions  Event_actions;

enum {
    USE_LEVEL_EVENT  = 0x00000001,
    USE_GREEDY_EVENT = 0x00000020,
    USE_EPOLL_EVENT  = 0x00000040,
    USE_FD_EVENT     = 0x00000400
};

struct EventConf : MConf {
    uint   connections;
    uint   use;
    flag_t multi_accept;
    const char* name;
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

/* for flags in (*process_events)() */
enum {
    UPDATE_TIME = 1,
    POST_EVENTS = 2
};

extern sig_atomic_t Event_timer_alarm;
extern bool Use_epoll_rdhup;
extern int Event_flags;

extern Module Events_module;
extern Module Event_core_module;

extern Os_io_t Io;

}

#define Get_event_conf(type, cycle, module)                                  \
    std::static_pointer_cast<type>(                                          \
        Get_conf(MConfs, cycle, Events_module)->ctxs[(module).ctx_index()]   \
    )

#endif
