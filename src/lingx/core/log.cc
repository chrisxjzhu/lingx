#include <lingx/core/log.h>
#include <lingx/core/cycle.h>
#include <lingx/core/module.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/open_file.h>
#include <lingx/core/path.h>     // is_relative()
#include <lingx/core/error.h>    // Strerrno()
#include <lingx/core/times.h>    // Cached_err_log_time
#include <lingx/core/strings.h>  // Slprintf()
#include <lingx/core/process_cycle.h>  // Pid
#include <lingx/config.h>        // LNX_ERROR_LOG_PATH
#include <cstdarg>
#include <unistd.h>              // write()
#include <fcntl.h>               // open()

namespace lnx {
namespace {

const char* Log_set_levels_(const Conf& cf, Log* log) noexcept;

const std::string_view Log_levels_[] = {
    "",
    "emerg",
    "alert",
    "crit",
    "error",
    "warn",
    "notice",
    "info",
    "debug"
};

std::vector<Command> Errlog_commands_ {
    Command {
        "error_log",
         MAIN_CONF|CONF_1MORE,
         Error_log,
         0
    }
};

CoreModuleCtx Errlog_module_ctx_ {
    "errlog",
    nullptr,
    nullptr
};

}

Module Errlog_module {
    "lnx_errlog_module",
    Errlog_module_ctx_,
    Errlog_commands_,
    CORE_MODULE,
    nullptr
};

bool Use_stderr = true;

LogPtr Init_new_log(const char* prefix)
{
    LogPtr log = std::make_shared<Log>();
    log->level_ = Log::NOTICE;

    const char* name = LNX_ERROR_LOG_PATH;
    size_t nlen = std::strlen(name);

    if (nlen == 0) {
        log->file_ = std::make_shared<OpenFile>(STDERR_FILENO, "");
        return log;
    }

    std::string path;
    if (Path(name).is_relative()) {
        path = prefix;
        Path::Tail_separator(path);
    }
    path += name;

    int fd = ::open(path.c_str(), O_WRONLY|O_CREAT|O_APPEND, 0644);
    if (fd == -1) {
        Log::Printf(errno, "[alert] could not open error log file: "
                           "open() \"%s\" failed", path.c_str());
        fd = STDERR_FILENO;
        path.clear();
    }

    log->file_ = std::make_shared<OpenFile>(fd, path);

    return log;
}

void Log::log(Level lvl, int err, const char* fmt, ...) const noexcept
{
    char errstr[MAX_ERROR_STR];
    const char* const last = errstr + sizeof(errstr) - sizeof('\n');

    char* p = Memcpy(errstr, Cached_err_log_time.data(),
                             Cached_err_log_time.size());

    p = Slprintf(p, last, " [%s] ", Log_levels_[lvl].data());

    /* TODO: enable tid */
    p = Slprintf(p, last, "%d#%d: ", Pid, 0);

    char* msg = p;

    va_list args;
    va_start(args, fmt);
    p = Vslprintf(p, last, fmt, args);
    va_end(args);

    if (err)
        p = Strerrno(p, last, err);

    *p++ = '\n';

    bool wrote_stderr = false;

    for (const Log* plog = this; plog; plog = plog->next_.get()) {

        if (plog->level_ < lvl)
            break;

        ::write(plog->file_->fd(), errstr, p - errstr);

        if (plog->file_->fd() == STDERR_FILENO)
            wrote_stderr = true;
    }

    if (!Use_stderr || lvl > WARN || wrote_stderr)
        return;

    const size_t len = 8 + Log_levels_[lvl].length() + 2;

    /* It's safe since  we have so much space before msg */
    msg -= len;

    std::snprintf(msg, len, "lingx: [%s] ", Log_levels_[lvl].data());

    /* restore the '\0' written by snprintf() to ' ' */
    msg[len-1] = ' ';

    ::write(STDERR_FILENO, msg, p - msg);
}

void Log::Printf(int err, const char* fmt, ...) noexcept
{
    char errstr[MAX_ERROR_STR];
    const char* const last = errstr + sizeof(errstr) - sizeof('\n');

    char* p = Memcpy(errstr, "lingx: ", 7);

    va_list args;
    va_start(args, fmt);
    p = Vslprintf(p, last, fmt, args);
    va_end(args);

    if (err)
        p = Strerrno(p, last, err);

    *p++ = '\n';

    ::write(STDERR_FILENO, errstr, p - errstr);
}

const Log* Get_file_log(const Log* head) noexcept
{
    for (auto log = head; log; log = log->next().get())
        if (log->file())
            return log;

    return nullptr;
}

const char* Error_log(const Conf& cf, const Command&, MConfPtr& /*conf*/)
{
    Log* dummy = &cf.cycle()->new_log_;

    return Log_set_log(cf, &dummy);
}

const char* Log_set_log(const Conf& cf, Log** head)
{
    LogPtr  new_logp;
    Log*    new_log;

    if (*head && (*head)->level() == Log::STDERR)
        new_log = *head;
    else {
        new_logp = std::make_shared<Log>();
        new_log = new_logp.get();

        if (*head == nullptr)
            *head = new_log;
    }

    const std::vector<std::string>& values = cf.args();

    if (values[1] == "stderr") {
        cf.cycle()->set_log_use_stderr(true);
        new_log->set_file(cf.cycle()->log_open_file(""));
    } else if (values[1].substr(0, 7) == "memory:") {
        cf.log_error(Log::EMERG, "lingx was built without debug support");
        return CONF_ERROR;
    } else if (values[1].substr(0, 7) == "syslog:") {
        cf.log_error(Log::EMERG, "not yet support");
        return CONF_ERROR;
    } else {
        new_log->set_file(cf.cycle()->log_open_file(values[1]));
    }

    if (Log_set_levels_(cf, new_log) != CONF_OK)
        return CONF_ERROR;

    if (*head != new_log)
        Log_insert(*head, new_logp);

    return CONF_OK;
}

void Log_insert(Log* log, const LogPtr& new_log) noexcept
{
    if (new_log->level_ > log->level_) {
        std::swap(*new_log, *log);
        log->next_ = new_log;
        return;
    }

    while (log->next_) {
        if (new_log->level_ > log->next_->level_) {
            new_log->next_ = log->next_;
            log->next_ = new_log;
            return;
        }

        log = log->next_.get();
    }

    log->next_ = new_log;
}

namespace {

const char* Log_set_levels_(const Conf& cf, Log* log) noexcept
{
    const std::vector<std::string>& values = cf.args();

    if (values.size() == 2) {
        log->set_level(Log::ERROR);
        return CONF_OK;
    }

    for (uint i = 2; i < values.size(); ++i) {
        bool found = false;

        for (uint n = 1; n <= Log::DEBUG; ++n) {
            if (values[i] == Log_levels_[n]) {

                if (log->level() != Log::STDERR) {
                    cf.log_error(Log::EMERG, "duplicate log level \"%s\"",
                                 values[i].c_str());
                    return CONF_ERROR;
                }

                log->set_level(Log::Level(n));

                found = true;
                break;
            }
        }

        if (!found) {
            cf.log_error(Log::EMERG, "invalid log level \"%s\"", values[i].c_str());
            return CONF_ERROR;
        }
    }

    return CONF_OK;
}

}

}
