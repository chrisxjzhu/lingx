#ifndef _LINGX_CORE_PROCESS_CYCLE_H
#define _LINGX_CORE_PROCESS_CYCLE_H

#include <lingx/core/common.h>

namespace lnx {

enum ProcessType {
    PROCESS_SINGLE,
    PROCESS_MASTER,
    PROCESS_SIGNALLER,
    PROCESS_WORKER,
    PROCESS_HELPER
};

void Master_process_cycle(const CyclePtr& cycle);
void Single_process_cycle(const CyclePtr& cycle);

extern ProcessType  Process_type;
extern pid_t        Pid;
extern bool         Daemonized;

extern bool Terminate;
extern bool Quit;

}

#endif
