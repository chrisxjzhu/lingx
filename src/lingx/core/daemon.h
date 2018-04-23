#ifndef _LINGX_CORE_DAEMON_H
#define _LINGX_CORE_DAEMON_H

#include <lingx/core/common.h>

namespace lnx {

rc_t Daemonize(const Log* log) noexcept;

}

#endif
