#include <lingx/core/cycle.h>
#include <lingx/core/times.h>
#include <lingx/core/path.h>
#include <lingx/core/file.h>
#include <lingx/core/module.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/connection.h>
#include <lingx/core/event.h>
#include <lingx/core/process.h>  // Os_signal_process()
#include <lingx/core/process_cycle.h>
#include <lingx/core/log.h>
#include <lingx/core/error.h>
#include <lingx/core/strings.h>  // Atoi()
#include <lingx/config.h>  // LNX_ERROR_LOG_PATH
#include <unistd.h>  // unlink()
#include <fcntl.h>   // open()

namespace lnx {

CyclePtr Cur_cycle = nullptr;

bool Opt_test_config = false;
bool Opt_quiet_mode  = false;

/* for initialize std::vector<Connection> and std::vector<Event> */
Cycle::Cycle() = default;
Cycle::~Cycle() = default;

void Cycle::init_connections_events()
{
    connections_.resize(connection_n_);

    read_events_.resize(connection_n_);
    for (auto& r : read_events_) {
        r.closed = 1;
        r.instance = 1;
    }

    write_events_.resize(connection_n_);
    for (auto& w : write_events_) {
        w.closed = 1;
    }

    Connection* c = connections_.data();
    void* next = nullptr;

    for (uint i = connection_n_; i; next = &c[i]) {
        --i;

        c[i].data = next;
        c[i].read = &read_events_[i];
        c[i].write = &write_events_[i];
        c[i].fd = -1;
    }

    free_connections_ = (Connection*) next;
    free_connection_n_ = connection_n_;
}

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

OpenFilePtr Cycle::log_open_file(const std::string& name)
{
    int fd = STDERR_FILENO;
    std::string fname;

    if (!name.empty()) {
        fname = Path::Get_full_name(prefix_, name);

        for (const auto& f : open_files_)
            if (f->name() == fname)
                return f;

        fd = ::open(fname.c_str(), O_WRONLY|O_APPEND|O_CREAT|O_CLOEXEC, 0644);
        if (fd == -1) {
            Log_error(log_, Log::EMERG, errno,
                      "open() \"%s\" failed", fname.c_str());
            return nullptr;
        }
    }

    for (const auto& f : open_files_)
        if (f->fd() == fd)
            return f;

    OpenFilePtr file = std::make_shared<OpenFile>(fd, fname);
    open_files_.push_back(file);

    return file;
}

rc_t Cycle::log_redirect_stderr() noexcept
{
     if (log_use_stderr_)
        return OK;

    int fd = log_->file()->fd();
    if (fd != STDERR_FILENO) {
        if (::dup2(fd, STDERR_FILENO) == -1) {
            Log_error(log_, Log::ALERT, errno, "dup2(, STDERR_FILENO) failed");
            return ERROR;
        }
    }

    return OK;
}

rc_t Cycle::log_open_default_()
{
    if (Get_file_log(&new_log_))
        return OK;

    LogPtr logp;
    Log*   log;

    if (new_log_.level() != Log::STDERR) {
        /* there are some error logs, but no files */
        logp = std::make_shared<Log>();
        log = logp.get();
    } else {
        log = &new_log_;
    }

    log->set_level(Log::ERROR);

    log->set_file(log_open_file(LNX_ERROR_LOG_PATH));

    if (log != &new_log_)
        Log_insert(&new_log_, logp);

    return OK;
}

rc_t Cycle::init_modules_()
{
    for (const Module& mod : modules_) {
        if (mod.init_module)
            if (mod.init_module(this) != OK)
                return ERROR;
    }

    return OK;
}

uint Cycle::count_modules(int type) const noexcept
{
    uint next = 0, max = 0;

    /* count appropriate modules, set up their indices */
    for (Module& mod : modules_) {
        if (mod.type() != type)
            continue;

        if (mod.ctx_index() != (uint) -1) {
            /* if ctx_index was assigned, preserve it */
            if (mod.ctx_index() > max)
                max = mod.ctx_index();

            if (mod.ctx_index() == next)
                ++next;

            continue;
        }

        /* search for some free index */
        mod.set_ctx_index(count_module_ctx_index_(type, next));

        if (mod.ctx_index() > max)
            max = mod.ctx_index();

        next = mod.ctx_index() + 1;
    }

    /*
     * make sure the number returned is big enough for previous
     * cycle as well, else there will be problems if the number
     * will be stored in a global variable (as it's used to be)
     * and we'll have to roll back to the previous cycle
     */

    if (old_cycle_) {
        for (const Module& mod : old_cycle_->modules_) {
            if (mod.type() != type)
                continue;

            if (mod.ctx_index() > max)
                max = mod.ctx_index();
        }
    }

    return max + 1;
}

uint Cycle::count_module_ctx_index_(int type, uint index) const noexcept
{
again:
    /* find an unused ctx_index */
    for (const Module& mod : modules_) {
        if (mod.type() != type)
            continue;

        if (mod.ctx_index() == index) {
            ++index;
            goto again;
        }
    }

    if (old_cycle_) {
        for (const Module& mod : old_cycle_->modules_) {
            if (mod.type() != type)
                continue;

            if (mod.ctx_index() == index) {
                ++index;
                goto again;
            }
        }
    }

    return index;
}

CyclePtr Init_new_cycle(const CyclePtr& old_cycle)
{
    Timezone_update();

    /* force localtime update with a new timezone */
    Cached_time->sec = 0;

    Time_update();

    const Log* log = old_cycle->log_;

    CyclePtr cycle = std::make_shared<Cycle>();

    cycle->log_ = log;
    cycle->old_cycle_ = old_cycle;

    cycle->prefix_ = old_cycle->prefix_;
    cycle->conf_prefix_ = old_cycle->conf_prefix_;
    cycle->conf_file_ = old_cycle->conf_file_;
    cycle->conf_param_ = old_cycle->conf_param_;

    cycle->conf_ctxs_.resize(Max_modules_n);

    cycle->modules_ = Modules;

    for (const Module& mod : cycle->modules_) {
        if (mod.type() != CORE_MODULE)
            continue;

        const CoreModuleCtx& ctx = static_cast<const CoreModuleCtx&>(mod.ctx());

        if (ctx.create_conf) {
            MConfPtr cf = ctx.create_conf(cycle.get());
            cycle->conf_ctxs_[mod.index()] = cf;
        }
    }

    Conf conf(cycle);
    conf.set_log(log);
    conf.set_ctxs(&cycle->conf_ctxs());
    conf.set_module_type(CORE_MODULE);
    conf.set_cmd_type(MAIN_CONF);

    if (conf.param() != CONF_OK)
        return nullptr;

    if (conf.parse(cycle->conf_file()) != CONF_OK)
        return nullptr;

    if (Opt_test_config && !Opt_quiet_mode) {
        Log::Printf(0, "the configuration file %s syntax is ok",
                    cycle->conf_file_.c_str());
    }

    for (const Module& mod : cycle->modules_) {
        if (mod.type() != CORE_MODULE)
            continue;

        const CoreModuleCtx& ctx = static_cast<const CoreModuleCtx&>(mod.ctx());

        if (ctx.init_conf) {
            if (ctx.init_conf(cycle.get(), cycle->conf_ctxs_[mod.index()].get())
                == CONF_ERROR)
            {
                return nullptr;
            }
        }
    }

    if (Process_type == PROCESS_SIGNALLER)
        return cycle;

    std::shared_ptr<CoreConf> ccf = Get_conf(CoreConf, cycle, Core_module);

    if (Opt_test_config) {
        if (Create_pidfile(ccf->pid_path, log) != OK)
            return nullptr;
    } else if (!old_cycle->is_init_cycle()) {
        /*
         * we do not create the pid file in the first Init_new_cycle() call
         * because we need to write the demonized process pid
         */
        std::shared_ptr<CoreConf> old_ccf = Get_conf(CoreConf, old_cycle, Core_module);

        if (ccf->pid_path != old_ccf->pid_path) {
            /* new pid file name */

            if (Create_pidfile(ccf->pid_path, log) != OK)
                return nullptr;

            Delete_pidfile(old_cycle);
        }
    }

    if (cycle->log_open_default_() != OK)
        return nullptr;

    cycle->log_ = &cycle->new_log_;

    /* commit the new cycle configuration */

    if (!Use_stderr)
        cycle->log_redirect_stderr();

    if (cycle->init_modules_() != OK)
        std::exit(1);

    /* close old open files */
    old_cycle->open_files_.clear();

    if (Process_type == PROCESS_MASTER || old_cycle->is_init_cycle()) {
        cycle->old_cycle_ = nullptr;
        return cycle;
    }

    return cycle;
}

int Signal_process(const CyclePtr& cycle, const char* sig) noexcept
{
    Log_error(cycle->log(), Log::NOTICE, 0, "signal process started");

    std::shared_ptr<CoreConf> ccf = Get_conf(CoreConf, cycle, Core_module);

    File file;

    if (file.open(ccf->pid_path.c_str(), O_RDONLY) == -1) {
        /* custom log level */
        Log_error(cycle->log(), Log::ERROR, errno,
                  "open() \"%s\" failed", ccf->pid_path.c_str());
        return 1;
    }

    char buf[MAX_INT_LEN + 2];

    ssize_t n = file.read(buf, sizeof(buf), 0);
    if (n <= 0) {
        Log_error(cycle->log(), Log::CRIT, errno,
                  "read() \"%s\" failed", ccf->pid_path.c_str());
        return 1;
    }

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

rc_t Create_pidfile(const std::string& path, const Log* log) noexcept
{
    if (Process_type > PROCESS_MASTER)
        return OK;

    File file;

    int flags = O_RDWR | O_CREAT;
    if (!Opt_test_config)
        flags |= O_TRUNC;

    if (file.open(path.c_str(), flags) == -1) {
        Log_error(log, Log::EMERG, errno, "open() \"%s\" failed", path.c_str());
        return ERROR;
    }

    if (!Opt_test_config) {
        char pid[MAX_INT_LEN + 2];
        int len = std::snprintf(pid, sizeof(pid), "%d\n", Pid);

        if (len <= 0 || (size_t) len >= sizeof(pid))
            return ERROR;

        if (file.write(pid, len, 0) == -1) {
            Log_error(log, Log::CRIT, errno,
                      "write() \"%s\" failed", path.c_str());
            return ERROR;
        }
    }

    return OK;
}

void Delete_pidfile(const CyclePtr& cycle) noexcept
{
    std::shared_ptr<CoreConf> ccf = Get_conf(CoreConf, cycle, Core_module);
    const char* path = ccf->pid_path.c_str();

    if (::unlink(path) == -1)
        Log_error(cycle->log(), Log::ALERT, errno,
                  "unlink() \"%s\" failed", path);
}

}
