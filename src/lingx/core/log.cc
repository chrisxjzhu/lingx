#include <lingx/core/log.h>
#include <lingx/core/open_file.h>
#include <lingx/core/path.h>     // is_relative()
#include <lingx/core/errno.h>    // Strerror()
#include <lingx/core/times.h>    // Cached_err_log_time
#include <lingx/core/strings.h>  // Slprintf()
#include <lingx/core/process_cycle.h>  // Pid
#include <lingx/config.h>        // LNX_ERROR_LOG_PATH
#include <cstdarg>
#include <unistd.h>              // write()
#include <fcntl.h>               // open()

namespace lnx {

const char* Err_levels_[] = {
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

Log::Log(const char* prefix)
{
    const char* name = LNX_ERROR_LOG_PATH;
    size_t nlen = std::strlen(name);

    if (nlen == 0) {
        file_ = std::make_shared<OpenFile>(STDERR_FILENO, false);
        return;
    }

    std::string path;
    if (Path(name).is_relative()) {
        path = prefix;
        Path::tail_separator(path);
    }
    path += name;

    int fd = ::open(path.c_str(), O_WRONLY|O_CREAT|O_APPEND, 0644);
    if (fd == -1) {
        Printf(errno, "[alert] could not open error log file: open()"
                      " \"%s\" failed", path.c_str());
        fd = STDERR_FILENO;
    }

    file_ = std::make_shared<OpenFile>(fd, (fd != STDERR_FILENO));
}

void Log::log(Level lvl, int err, const char* fmt, ...) noexcept
{
    char errstr[MAX_ERROR_STR];
    const char* last = errstr + sizeof(errstr) - sizeof('\n');

    char* p = Memcpy(errstr, Cached_err_log_time.data(),
                             Cached_err_log_time.size());

    p = Slprintf(p, last, " [%s] ", &Err_levels_[lvl]);

    /* TODO: enable tid */
    p = Slprintf(p, last, "%d#%d: ", Pid, 0);

    va_list args;
    va_start(args, fmt);
    p = Vslprintf(p, last, fmt, args);
    va_end(args);

    if (err)
        p = Strerrno_(p, last, err);

    *p++ = '\n';

    ::write(file_->fd(), errstr, p - errstr);
}

void Log::Printf(int err, const char* fmt, ...) noexcept
{
    char errstr[MAX_ERROR_STR];
    const char* last = errstr + sizeof(errstr) - sizeof('\n');

    char* p = Memcpy(errstr, "lingx: ", 7);

    va_list args;
    va_start(args, fmt);
    p = Vslprintf(p, last, fmt, args);
    va_end(args);

    if (err)
        p = Strerrno_(p, last, err);

    *p++ = '\n';

    ::write(STDERR_FILENO, errstr, p - errstr);
}

char* Log::Strerrno_(char* buf, const char* last, int err) noexcept
{
    if (buf + 50 > last) {
        /* leave space for errno message 
         *
         * WARN: we must ensure the total buffer size is larger than 50 chars.
         * So we should always use a char[MAX_ERROR_STR] with this function.
         */
        buf = const_cast<char*>(last) - 50;

        for (int i = 0; i < 3; ++i)
            *buf++ = '.';
    }

    buf = Slprintf(buf, last, " (%d: ", err);

    buf = Strerror(err, buf, last - buf);

    if (buf < last)
        *buf++ = ')';

    return buf;
}

}
