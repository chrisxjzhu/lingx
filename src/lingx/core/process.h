#ifndef _LINGX_CORE_PROCESS_H
#define _LINGX_CORE_PROCESS_H

#include <lingx/core/common.h>
#include <sys/types.h>

namespace lnx {

int Os_signal_process(const CyclePtr& cycle, const char* name, pid_t pid) noexcept;

}

#endif
