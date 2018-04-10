#ifndef _LINGX_CORE_LOG_H
#define _LINGX_CORE_LOG_H

#include <memory>

namespace lnx {

class OpenFile;

class Log {
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

    explicit Log(const char* prefix);

    Level level() const noexcept
    { return level_; }

    void set_level(Level lvl) noexcept
    { level_ = lvl; }

    void log(Level lvl, int err, const char* fmt, ...) noexcept;

    static const size_t MAX_ERROR_STR = 1024;

    static void Printf(int err, const char* fmt, ...) noexcept;

private:
    static char* Strerrno_(char* buf, const char* last, int err) noexcept;

    Level level_ = Level::NOTICE;
    std::shared_ptr<OpenFile> file_;
};

}

#define lnx_Log_error(plog, lvl, err, ...)                                   \
    do {                                                                     \
        if ((plog)->level() >= lvl) (plog)->log(lvl, err, __VA_ARGS__);      \
    } while (0)

#endif
