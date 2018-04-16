#include <lingx/core/process_cycle.h>
#include <lingx/core/cycle.h>
#include <lingx/core/log.h>

namespace lnx {

ProcessType  Process_type = PROCESS_SINGLE;
pid_t        Pid = -1;
bool         Daemonized = false;

void Master_process_cycle(const CyclePtr&)
{
}

void Single_process_cycle(const CyclePtr&)
{
}

}
