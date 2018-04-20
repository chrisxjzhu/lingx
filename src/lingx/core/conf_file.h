#ifndef _LINGX_CORE_CONF_FILE_H
#define _LINGX_CORE_CONF_FILE_H

#include <lingx/core/common.h>
#include <lingx/core/file.h> // File
#include <lingx/core/log.h>  // Log::Level

namespace lnx {

/*
 *        AAAA  number of arguments
 *      FF      command flags
 *    TT        command type, i.e. HTTP "location" or "server" command
 */
enum CommandType {
    CONF_NOARGS   = 0x00000001,
    CONF_TAKE1    = 0x00000002,
    CONF_TAKE2    = 0x00000004,
    CONF_TAKE3    = 0x00000008,
    CONF_TAKE4    = 0x00000010,
    CONF_TAKE5    = 0x00000020,
    CONF_TAKE6    = 0x00000040,
    CONF_TAKE7    = 0x00000080,

    CONF_TAKE12   = (CONF_TAKE1|CONF_TAKE2),
    CONF_TAKE13   = (CONF_TAKE1|CONF_TAKE3),
    CONF_TAKE23   = (CONF_TAKE2|CONF_TAKE3),
    CONF_TAKE123  = (CONF_TAKE1|CONF_TAKE2|CONF_TAKE3),
    CONF_TAKE1234 = (CONF_TAKE1|CONF_TAKE2|CONF_TAKE3|CONF_TAKE4),

    CONF_BLOCK    = 0x00000100,
    CONF_FLAG     = 0x00000200,
    CONF_ANY      = 0x00000400,
    CONF_1MORE    = 0x00000800,
    CONF_2MORE    = 0x00001000,

    DIRECT_CONF   = 0x00010000,

    MAIN_CONF     = 0x01000000,
    EVENT_CONF    = 0x02000000,
    ANY_CONF      = 0x1F000000
};

const size_t CONF_MAX_ARGS = 8;

const char* const CONF_OK = nullptr;
const char* const CONF_ERROR = (const char*) -1;

const flag_t OFF = 0;
const flag_t ON  = 1;

const int UNSET  = -1;

class Conf;

struct Command {
    std::string_view name;
    int type = 0;
    const char *(*set)(const Conf&, const Command&, MConfPtr&);
    size_t offset;
};

const size_t MAX_CONF_ERRSTR = 1024;

struct Buf;

struct ConfFile {
    File  file;
    Buf*  buffer;
    uint  line;
};

class Conf {
public:
    explicit Conf(const CyclePtr& cycle) noexcept
        : cycle_(cycle.get())
    { }

    Cycle* cycle() const noexcept
    { return cycle_; }

    void set_ctxs(std::vector<MConfPtr>* ctxs) noexcept
    { ctxs_ = ctxs; }

    void set_module_type(int type) noexcept
    { module_type_ = type; }

    void set_cmd_type(int type) noexcept
    { cmd_type_ = type; }

    void set_log(const LogPtr& log) noexcept
    { log_ = log.get(); }

    const char* param() noexcept;
    const char* parse(const std::string& filename) noexcept;

    void log_error(Log::Level lvl, const char* fmt, ...) const noexcept;

    const char* parse_flag_arg(const char* cmd, flag_t* fp) const noexcept;

private:
    int read_token_() noexcept;
    rc_t handle_(int last) noexcept;

    std::vector<std::string> args_;
    Cycle* cycle_ = nullptr;
    ConfFile* conf_file_ = nullptr;

    std::vector<MConfPtr>* ctxs_ = nullptr;
    int module_type_ = 0;
    int cmd_type_ = 0;

    Log* log_ = nullptr;
};

template <typename T>
void Conf_init_value(T& conf, T val)
{
    if (conf == (T) UNSET)
        conf = val;
}

const char* Set_flag_slot(const Conf& cf, const Command& cmd, MConfPtr& conf);

}

#endif
