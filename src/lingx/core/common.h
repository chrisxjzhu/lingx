#ifndef _LINGX_CORE_COMMON_H
#define _LINGX_CORE_COMMON_H

#include <memory>
#include <vector>
#include <string>
#include <experimental/string_view>
namespace std { using experimental::string_view; }
#include <cstring>
#include <sys/types.h>

namespace lnx {

enum rc_t {
    OK       =  0,
    ERROR    = -1,
    AGAIN    = -2,
    BUSY     = -3,
    DONE     = -4,
    DECLINED = -5,
    ABORT    = -6
};

typedef uint flag_t;

typedef ulong msec_t;
typedef long  signed_msec_t;

constexpr size_t MAX_INT_LEN   = sizeof("-2147483648") - 1;
constexpr size_t MAX_LONG_LEN  = sizeof("-9223372036854775808") - 1;

class Log;
using LogPtr = std::shared_ptr<Log>;

class Cycle;
using CyclePtr = std::shared_ptr<Cycle>;

class Conf;

struct MConf {};
using  MConfPtr = std::shared_ptr<MConf>;

using ConfCtx = std::vector<MConfPtr>;
using ConfCtxPtr = std::shared_ptr<ConfCtx>;

struct EventConfCtx : MConf {
    ConfCtxPtr conf;
};

struct HttpConfCtx : MConf {
    ConfCtxPtr main_conf;
    ConfCtxPtr srv_conf;
    ConfCtxPtr loc_conf;
};

struct Command;

struct ModuleCtx {};

class Module;

class OpenFile;
using OpenFilePtr = std::shared_ptr<OpenFile>;

struct Event;
struct Connection;

}

#endif
