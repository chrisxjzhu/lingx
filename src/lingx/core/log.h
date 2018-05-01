#ifndef _LINGX_CORE_LOG_H
#define _LINGX_CORE_LOG_H

#include <lingx/core/common.h>

namespace lnx {

class Log {
    friend LogPtr Init_new_log(const char* prefix);
    friend void Log_insert(Log* log, const LogPtr& new_log) noexcept;

public:
    enum Level {
        STDERR = 0,
        EMERG  = 1,
        ALERT  = 2,
        CRIT   = 3,
        ERROR  = 4,
        WARN   = 5,
        NOTICE = 6,
        INFO   = 7,
        DEBUG  = 8,

        DEBUG_CORE   = 0x010,
        DEBUG_ALLOC  = 0x020,
        DEBUG_MUTEX  = 0x040,
        DEBUG_EVENT  = 0x080,
        DEBUG_HTTP   = 0x100,
        DEBUG_MAIL   = 0x200,
        DEBUG_STREAM = 0x400,

        DEBUG_FIRST      = DEBUG_CORE,
        DEBUG_LAST       = DEBUG_STREAM,
        DEBUG_CONNECTION = 0x80000000,
        DEBUG_ALL        = 0x7ffffff0
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

    void set_debug_level(Level lvl) noexcept
    { level_ = Level(level_ | lvl); }

    void set_file(const OpenFilePtr& file) noexcept
    { file_ = file; }

    void log(Level lvl, int err, const char* fmt, ...) const noexcept;

    static void Printf(int err, const char* fmt, ...) noexcept;

private:
    Level level_ = STDERR;
    OpenFilePtr file_;

    LogPtr next_;
};

LogPtr Init_new_log(const char* prefix);

const Log* Get_file_log(const Log* head) noexcept;

const char* Error_log(const Conf& cf, const Command& cmd, MConfPtr& conf);

const char* Log_set_log(const Conf& cf, Log** head);

void Log_insert(Log* log, const LogPtr& new_log) noexcept;

extern bool Use_stderr;

}

#define Log_error(plog, lvl, err, ...)                                       \
    do {                                                                     \
        if ((plog) && (plog)->level() >= lvl)                                \
            (plog)->log(lvl, err, __VA_ARGS__);                              \
    } while (0)

#define Log_debug(plog, lvl, err, ...)                                       \
    do {                                                                     \
        if ((plog) && (plog)->level() & lvl)                                 \
            (plog)->log(Log::DEBUG, err, __VA_ARGS__);                       \
    } while (0)

#endif
