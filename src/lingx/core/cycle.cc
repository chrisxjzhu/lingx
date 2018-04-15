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
#include <unistd.h>  // unlink()

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

    std::shared_ptr<CoreConf> ccf = Get_module_conf(CoreConf, cycle, Core_module);

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

    std::shared_ptr<CoreConf> ccf = Get_module_conf(CoreConf, cycle, Core_module);

    File file;

    if (file.open(ccf->pid_path.c_str(), O_RDONLY) == -1) {
        /* custom log level */
        Log_error(cycle->log(), Log::ERROR, errno,
                  "open() \"%s\" failed", ccf->pid_path.c_str());
        return 1;
    }

    file.set_log(cycle->log());

    char buf[MAX_INT_LEN + 2];

    ssize_t n = file.read(buf, sizeof(buf), 0);
    if (n <= 0)
        return 1;

    /* close it early */
    file.close();

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

rc_t Create_pidfile(const std::string& path, const LogPtr& log) noexcept
{
    if (Process_type > PROCESS_MASTER)
        return LNX_OK;

    File file(log);

    int flags = O_RDWR | O_CREAT;
    if (!Opt_test_config)
        flags |= O_TRUNC;

    if (file.open(path.c_str(), flags) == -1)
        return LNX_ERROR;

    if (!Opt_test_config) {
        char pid[MAX_INT_LEN + 2];
        int len = std::snprintf(pid, sizeof(pid), "%d\n", Pid);

        if (len <= 0 || (size_t) len >= sizeof(pid))
            return LNX_ERROR;

        if (file.write(pid, len, 0) == -1)
            return LNX_ERROR;
    }

    return LNX_OK;
}

void Delete_pidfile(const CyclePtr& cycle) noexcept
{
    std::shared_ptr<CoreConf> ccf = Get_module_conf(CoreConf, cycle, Core_module);
    const char* path = ccf->pid_path.c_str();

    if (::unlink(path) == -1)
        Log_error(cycle->log(), Log::ALERT, errno,
                  "unlink() \"%s\" failed", path);
}

}
