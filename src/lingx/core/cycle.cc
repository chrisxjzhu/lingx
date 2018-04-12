#include <lingx/core/cycle.h>
#include <lingx/core/times.h>
#include <lingx/core/path.h>
#include <lingx/core/module.h>
#include <lingx/core/process_cycle.h>

namespace lnx {

CyclePtr Cur_cycle = nullptr;

bool Opt_test_config = false;

void Cycle::set_prefix(const char* prefix)
{
    prefix_ = prefix;
    Path::tail_separator(prefix_);
}

void Cycle::set_conf_file(const char* file)
{
    conf_file_ = file;
    conf_prefix_ = Path(file).parent_path().string();
    Path::tail_separator(conf_prefix_);
}

CyclePtr Init_new_cycle(const CyclePtr& old_cycle)
{
    Timezone_update();

    /* force localtime update with a new timezone */
    Cached_time->sec = 0;

    Time_update();

    CyclePtr cycle = std::make_shared<Cycle>();

    cycle->log_ = old_cycle->log_;
    cycle->prefix_ = old_cycle->prefix_;
    cycle->conf_prefix_ = old_cycle->conf_prefix_;
    cycle->conf_file_ = old_cycle->conf_file_;
    cycle->conf_param_ = old_cycle->conf_param_;

    cycle->conf_ctx_.resize(Max_modules_n);

    cycle->modules_ = Modules;

    for (Module& mod : cycle->modules_) {
        if (mod.type() != CORE_MODULE)
            continue;

        const CoreModuleCtx& ctx = static_cast<const CoreModuleCtx&>(mod.ctx());

        if (ctx.create_conf) {
            MConfPtr cf = ctx.create_conf(cycle);
            cycle->conf_ctx_[mod.index()] = cf;
        }
    }

    for (Module& mod : cycle->modules_) {
        if (mod.type() != CORE_MODULE)
            continue;

        const CoreModuleCtx& ctx = static_cast<const CoreModuleCtx&>(mod.ctx());

        if (ctx.init_conf) {
            if (ctx.init_conf(cycle, cycle->conf_ctx_[mod.index()])
                == LNX_CONF_ERROR)
            {
                return nullptr;
            }
        }
    }

    if (Process_type == PROCESS_SIGNALLER)
        return cycle;

    std::shared_ptr<CoreConf> ccf = std::static_pointer_cast<CoreConf>
                                    (cycle->conf_ctx_[Core_module.index()]);

    if (Opt_test_config) {
        /* TODO: create pidfile */
    } else if (!old_cycle->is_init_cycle()) {
        /*
         * we do not create the pid file in the first ngx_init_cycle() call
         * because we need to write the demonized process pid
         */
    }

    return cycle;
}

}
