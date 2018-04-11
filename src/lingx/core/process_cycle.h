#ifndef _LINGX_CORE_PROCESS_CYCLE_H
#define _LINGX_CORE_PROCESS_CYCLE_H

#include <sys/types.h>

namespace lnx {

enum ProcessType {
    PROCESS_SINGLE,
    PROCESS_MASTER,
    PROCESS_SIGNALLER,
    PROCESS_WORKER,
    PROCESS_HELPER
};

extern ProcessType  Process_type;
extern pid_t        Pid;

}

#endif
