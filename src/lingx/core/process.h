#ifndef _LINGX_CORE_PROCESS_H
#define _LINGX_CORE_PROCESS_H

#include <lingx/core/common.h>

namespace lnx {

rc_t Init_signals(const Log* log) noexcept;

int Os_signal_process(const CyclePtr& cycle, const char* name, pid_t pid) noexcept;

}

#endif
