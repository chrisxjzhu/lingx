#include <lingx/core/cycle.h>
#include <lingx/core/times.h>
#include <lingx/core/path.h>
#include <lingx/core/file.h>
#include <lingx/core/module.h>
#include <lingx/core/process.h>  // Os_signal_process()
#include <lingx/core/process_cycle.h>
#include <lingx/core/log.h>
#include <lingx/core/error.h>
#include <lingx/core/strings.h>  // Atoi()

namespace lnx {

CyclePtr Cur_cycle = nullptr;

bool Opt_test_config = false;

void Cycle::set_prefix(const char* prefix)
{
    prefix_ = prefix;
    Path::Tail_separator(prefix_);
}

void Cycle::set_conf_file(const std::string& file)
{
    conf_file_ = file;
    conf_prefix_ = Path(file).parent_path().string();
    Path::Tail_separator(conf_prefix_);
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

int Signal_process(const CyclePtr& cycle, const char* sig) noexcept
{
    Log_error(cycle->log(), Log::NOTICE, 0, "signal process started");

    std::shared_ptr<CoreConf> ccf = std::static_pointer_cast<CoreConf>
                                    (cycle->conf_ctx()[Core_module.index()]);

    char buf[MAX_INT_LEN + 2];
    ssize_t n = 0;

    /* TODO: handle File ops and logging consistently */
    try {
        File file(ccf->pid_path.c_str(), O_RDONLY);
        file.set_log(cycle->log());
        n = file.read(buf, sizeof(buf), 0);
    } catch (const Error& err) {
        Log_error(cycle->log(), Log::ERROR, 0, err.what());
        return 1;
    }

    if (n <= 0)
        return 1;

    while (n > 0 && (buf[n-1] == '\r' || buf[n-1] == '\n'))
        --n;

    pid_t pid = Atoi(buf, n);
    if (pid == -1) {
        Log_error(cycle->log(), Log::ERROR, 0,
                  "invalid PID number \"%.*s\" in \"%s\"",
                  n, buf, ccf->pid_path.c_str());
        return 1;
    }

    return Os_signal_process(cycle, sig, pid);
}

}
