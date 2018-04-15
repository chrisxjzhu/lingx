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
    LNX_OK       =  0,
    LNX_ERROR    = -1,
    LNX_AGAIN    = -2,
    LNX_BUSY     = -3,
    LNX_DONE     = -4,
    LNX_DECLINED = -5,
    LNX_ABORT    = -6
};

constexpr size_t MAX_INT_LEN   = sizeof("-2147483648") - 1;
constexpr size_t MAX_LONG_LEN  = sizeof("-9223372036854775808") - 1;

class Log;
using LogPtr = std::shared_ptr<Log>;

class Cycle;
using CyclePtr = std::shared_ptr<Cycle>;

struct MConf {};
using  MConfPtr = std::shared_ptr<MConf>;

/* temporarily put here */
const char* const LNX_CONF_OK = nullptr;
const char* const LNX_CONF_ERROR = (const char*) -1;

/* temporarily put here */
#define Get_module_conf(type, cycle, module)                                 \
    std::static_pointer_cast<type>((cycle)->conf_ctx()[(module).index()])

}

#endif
