#ifndef _LINGX_CORE_COMMON_H
#define _LINGX_CORE_COMMON_H

#include <memory>
#include <vector>
#include <string>
#include <experimental/string_view>
namespace std { using experimental::string_view; }

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

using uint = unsigned int;
using ulong = unsigned long;

class Log;
using LogPtr = std::shared_ptr<Log>;

class Cycle;

struct MConf {};
using  MConfPtr = std::shared_ptr<MConf>;

/* temporarily put here */
const char* const LNX_CONF_OK = nullptr;
const char* const LNX_CONF_ERROR = (const char*) -1;

}

#endif
