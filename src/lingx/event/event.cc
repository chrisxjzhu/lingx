#include <lingx/event/event.h>
#include <lingx/core/module.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/cycle.h>
#include <lingx/core/log.h>

namespace lnx {
namespace {

const char* Events_block_(const Conf& cf, const Command& cmd, MConfPtr& conf);

const char* Events_init_conf_(const CyclePtr& cycle, const MConfPtr& conf);

std::vector<Command> Events_commands_ {
    Command {
        "events",
        MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
        Events_block_,
        0
    }
};

CoreModuleCtx Events_module_ctx_ {
    "events",
    nullptr,
    Events_init_conf_
};

}

Module Events_module {
    "lnx_events_module",
    Events_module_ctx_,
    Events_commands_,
    CORE_MODULE
};

namespace {

const char* Events_block_(const Conf& cf, const Command&, MConfPtr& conf)
{
    if (conf.get())
        return "is duplicate";

    /* count the number of the event modules and set up their indices */
    uint event_max_module = cf.cycle()->count_modules(EVENT_MODULE);

    std::shared_ptr<MConfs> econfs = std::make_shared<MConfs>();
    econfs->ctxs.resize(event_max_module);

    conf = econfs;

    for (const Module& mod : cf.cycle()->modules()) {
        if (mod.type() != EVENT_MODULE)
            continue;

        const EventModuleCtx& ctx = static_cast<const EventModuleCtx&>(mod.ctx());

        if (ctx.create_conf) {
            econfs->ctxs[mod.ctx_index()] = ctx.create_conf(cf.cycle());
            if (econfs->ctxs[mod.ctx_index()] == nullptr)
                return CONF_ERROR;
        }
    }

    Conf ncf = cf;
    ncf.set_ctxs(&econfs->ctxs);
    ncf.set_module_type(EVENT_MODULE);
    ncf.set_cmd_type(EVENT_CONF);

    const char* rv = ncf.parse("");
    if (rv != CONF_OK)
        return rv;

    for (const Module& mod : cf.cycle()->modules()) {
        if (mod.type() != EVENT_MODULE)
            continue;

        const EventModuleCtx& ctx = static_cast<const EventModuleCtx&>(mod.ctx());

        if (ctx.init_conf) {
            rv = ctx.init_conf(cf.cycle(), econfs->ctxs[mod.ctx_index()]);
            if (rv != CONF_OK)
                return rv;
        }
    }

    return CONF_OK;
}

const char* Events_init_conf_(const CyclePtr& cycle, const MConfPtr&)
{
    if (Get_module_conf(MConfs, cycle, Events_module) == nullptr) {
        Log_error(cycle->log(), Log::EMERG, 0,
                  "no \"events\" section in configuration");
        return CONF_ERROR;
    }

    return CONF_OK;
}

}

}
