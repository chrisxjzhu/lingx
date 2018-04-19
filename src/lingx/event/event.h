#ifndef _LINGX_EVENT_EVENT_H
#define _LINGX_EVENT_EVENT_H

#include <lingx/core/common.h>

namespace lnx {

struct EventActions {
};

struct EventConf : MConf {
    uint   connections;
    flag_t multi_accept;
};

struct EventModuleCtx : ModuleCtx {
    typedef std::function<MConfPtr(const CyclePtr&)> create_conf_t;
    typedef std::function<const char*(const CyclePtr&, const MConfPtr&)> init_conf_t;

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

}

#endif
