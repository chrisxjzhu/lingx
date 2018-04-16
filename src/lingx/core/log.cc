#include <lingx/core/log.h>
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

bool Use_stderr = true;

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

void Log::log(Level lvl, int err, const char* fmt, ...) noexcept
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

    for (Log* plog = this; plog; plog = plog->next_.get()) {

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

LogPtr Get_file_log(const LogPtr& head) noexcept
{
    for (auto log = head; log; log = log->next())
        if (log->file())
            return log;

    return nullptr;
}

void Log_insert(LogPtr log, const LogPtr& new_log) noexcept
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

        log = log->next_;
    }

    log->next_ = new_log;
}

}
