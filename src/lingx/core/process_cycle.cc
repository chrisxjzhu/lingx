#include <lingx/core/process_cycle.h>
#include <lingx/core/cycle.h>
#include <lingx/core/event.h>
#include <lingx/core/module.h>
#include <lingx/core/log.h>

namespace lnx {
namespace {

void Master_process_exit_(const CyclePtr& cycle);

}

ProcessType  Process_type = PROCESS_SINGLE;
pid_t        Pid = -1;

bool Terminate = false;
bool Quit = false;

bool Daemonized = false;

void Master_process_cycle(const CyclePtr&)
{
}

void Single_process_cycle(const CyclePtr& cycle)
{
    for (const Module& mod : cycle->modules()) {
        if (mod.init_process)
            if (mod.init_process(cycle.get()) == ERROR)
                std::exit(2);
    }

    for ( ;; ) {
        Log_error(cycle->log(), Log::DEBUG, 0, "worker cycle");

        Process_events_and_timers(cycle);

        if (Terminate || Quit) {
            Master_process_exit_(cycle);
        }
    }
}

namespace {

void Master_process_exit_(const CyclePtr& cycle)
{
    Delete_pidfile(cycle);

    Log_error(cycle->log(), Log::NOTICE, 0, "exit");

    std::exit(0);
}

}
}
