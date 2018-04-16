#ifndef _LINGX_CORE_LOG_H
#define _LINGX_CORE_LOG_H

#include <lingx/core/common.h>

namespace lnx {

class Log {
    friend LogPtr Init_new_log(const char* prefix);
    friend void Log_insert(LogPtr log, const LogPtr& new_log) noexcept;

public:
    enum Level {
        STDERR = 0,
        EMERG,
        ALERT,
        CRIT,
        ERROR,
        WARN,
        NOTICE,
        INFO,
        DEBUG
    };

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    Log() = default;

    Log(Log&&) = default;
    Log& operator=(Log&&) = default;

    Level level() const noexcept
    { return level_; }

    const OpenFilePtr& file() const noexcept
    { return file_; }

    const LogPtr& next() const noexcept
    { return next_; }

    void set_level(Level lvl) noexcept
    { level_ = lvl; }

    void set_file(const OpenFilePtr& file) noexcept
    { file_ = file; }

    void log(Level lvl, int err, const char* fmt, ...) noexcept;

    static void Printf(int err, const char* fmt, ...) noexcept;

private:
    Level level_ = STDERR;
    OpenFilePtr file_;

    LogPtr next_;
};

LogPtr Init_new_log(const char* prefix);

LogPtr Get_file_log(const LogPtr& head) noexcept;

void Log_insert(LogPtr log, const LogPtr& new_log) noexcept;

extern bool Use_stderr;

}

#define Log_error(plog, lvl, err, ...)                                       \
    do {                                                                     \
        if ((plog) && (plog)->level() >= lvl)                                \
            (plog)->log(lvl, err, __VA_ARGS__);                              \
    } while (0)

#endif
