#ifndef _LINGX_CORE_COMMON_H
#define _LINGX_CORE_COMMON_H

#include <memory>
#include <experimental/string_view>
namespace std { using experimental::string_view; }

namespace lnx {

using uint = unsigned int;
using ulong = unsigned long;

class Log;
using LogPtr = std::shared_ptr<Log>;

}

#endif
